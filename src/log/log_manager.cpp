#include "log_manager.h"

#include <array>
#include <cstring>
#include <stdexcept>

namespace flashdb {

namespace {

constexpr std::uint32_t TRANSACTION_LOG_VERSION = 1;
constexpr std::uint32_t UNDO_LOG_VERSION = 1;

void append_u32(
    std::vector<std::byte>& output,
    std::uint32_t value
) {
    for (int i = 0; i < 4; ++i) {
        output.push_back(
            static_cast<std::byte>(
                (value >> (i * 8)) & 0xff
            )
        );
    }
}

void append_u64(
    std::vector<std::byte>& output,
    std::uint64_t value
) {
    for (int i = 0; i < 8; ++i) {
        output.push_back(
            static_cast<std::byte>(
                (value >> (i * 8)) & 0xff
            )
        );
    }
}

void append_string(
    std::vector<std::byte>& output,
    const std::string& value
) {
    append_u32(
        output,
        static_cast<std::uint32_t>(value.size())
    );

    for (char character : value) {
        output.push_back(
            static_cast<std::byte>(
                static_cast<unsigned char>(character)
            )
        );
    }
}

std::uint32_t read_u32(
    const std::vector<std::byte>& input,
    std::size_t& offset
) {
    if (offset + 4 > input.size()) {
        throw std::runtime_error("Invalid log record");
    }

    std::uint32_t value = 0;

    for (int i = 0; i < 4; ++i) {
        value |=
            static_cast<std::uint32_t>(
                std::to_integer<unsigned char>(
                    input[offset++]
                )
            ) << (i * 8);
    }

    return value;
}

std::uint64_t read_u64(
    const std::vector<std::byte>& input,
    std::size_t& offset
) {
    if (offset + 8 > input.size()) {
        throw std::runtime_error("Invalid log record");
    }

    std::uint64_t value = 0;

    for (int i = 0; i < 8; ++i) {
        value |=
            static_cast<std::uint64_t>(
                std::to_integer<unsigned char>(
                    input[offset++]
                )
            ) << (i * 8);
    }

    return value;
}

void append_int(
    std::vector<std::byte>& output,
    int value
) {
    append_u32(
        output,
        static_cast<std::uint32_t>(value)
    );
}

int read_int(
    const std::vector<std::byte>& input,
    std::size_t& offset
) {
    return static_cast<int>(
        read_u32(input, offset)
    );
}

std::string read_string(
    const std::vector<std::byte>& input,
    std::size_t& offset
) {
    const std::uint32_t length = read_u32(
        input,
        offset
    );

    if (offset + length > input.size()) {
        throw std::runtime_error("Invalid log string");
    }

    std::string value;

    value.reserve(length);

    for (std::uint32_t i = 0; i < length; ++i) {
        value.push_back(
            static_cast<char>(
                std::to_integer<unsigned char>(
                    input[offset++]
                )
            )
        );
    }

    return value;
}

void append_string_map(
    std::vector<std::byte>& output,
    const std::unordered_map<std::string, std::string>& values
) {
    append_u32(
        output,
        static_cast<std::uint32_t>(values.size())
    );

    for (const auto& [name, value] : values) {
        append_string(output, name);
        append_string(output, value);
    }
}

std::unordered_map<std::string, std::string> read_string_map(
    const std::vector<std::byte>& input,
    std::size_t& offset
) {
    const std::uint32_t count = read_u32(
        input,
        offset
    );

    std::unordered_map<std::string, std::string> values;

    for (std::uint32_t i = 0; i < count; ++i) {
        const std::string name = read_string(
            input,
            offset
        );

        const std::string value = read_string(
            input,
            offset
        );

        if (!values.emplace(name, value).second) {
            throw std::runtime_error(
                "Duplicate field in undo record"
            );
        }
    }

    return values;
}

} // namespace

// Open the database log and recover the number of existing records.
LogManager::LogManager(
    const std::string& database_directory
)
    : log_path_(
        std::filesystem::path(database_directory) /
        "flashdb.log"
    ),
      record_count_(0) {
    std::filesystem::create_directories(
        database_directory
    );

    record_count_ = count_existing_records();

    log_file_.open(
        log_path_,
        std::ios::binary |
        std::ios::app
    );

    if (!log_file_) {
        throw std::runtime_error(
            "Failed to open log file"
        );
    }
}

// Append one physical log record and return its LSN.
std::size_t LogManager::append(
    const std::vector<std::byte>& record
) {
    const std::uint32_t length =
        static_cast<std::uint32_t>(record.size());

    log_file_.write(
        reinterpret_cast<const char*>(&length),
        sizeof(length)
    );

    if (!record.empty()) {
        log_file_.write(
            reinterpret_cast<const char*>(record.data()),
            static_cast<std::streamsize>(record.size())
        );
    }

    if (!log_file_) {
        throw std::runtime_error(
            "Failed to append log record"
        );
    }

    return record_count_++;
}

// Write a transaction lifecycle record into the WAL.
std::size_t LogManager::append_transaction_record(
    std::size_t transaction_id,
    LogRecordType type
) {
    return append(
        encode_transaction_record(
            transaction_id,
            type
        )
    );
}

// Write enough information to undo one data-page modification.
std::size_t LogManager::append_undo_record(
    const UndoLogRecord& record
) {
    return append(
        encode_undo_record(record)
    );
}

// Read a physical WAL record by LSN.
std::vector<std::byte> LogManager::read(
    std::size_t lsn
) const {
    std::ifstream input(
        log_path_,
        std::ios::binary
    );

    if (!input) {
        throw std::runtime_error(
            "Failed to open log file"
        );
    }

    std::size_t current = 0;

    while (true) {
        std::uint32_t length = 0;

        input.read(
            reinterpret_cast<char*>(&length),
            sizeof(length)
        );

        if (!input) {
            break;
        }

        std::vector<std::byte> record(length);

        if (length > 0) {
            input.read(
                reinterpret_cast<char*>(record.data()),
                static_cast<std::streamsize>(length)
            );

            if (!input) {
                throw std::runtime_error(
                    "Incomplete log record"
                );
            }
        }

        if (current == lsn) {
            return record;
        }

        ++current;
    }

    throw std::out_of_range(
        "Log sequence number does not exist"
    );
}

// Read and decode a transaction lifecycle record by LSN.
TransactionLogRecord LogManager::read_transaction_record(
    std::size_t lsn
) const {
    return decode_transaction_record(
        read(lsn)
    );
}

// Read and decode an undo record by LSN.
UndoLogRecord LogManager::read_undo_record(
    std::size_t lsn
) const {
    return decode_undo_record(
        read(lsn)
    );
}

// Make all currently buffered WAL data durable.
void LogManager::flush() {
    log_file_.flush();

    if (!log_file_) {
        throw std::runtime_error(
            "Failed to flush log"
        );
    }
}

// Return the number of physical WAL records.
std::size_t LogManager::size() const {
    return record_count_;
}

// Count complete records already present when the database starts.
std::size_t LogManager::count_existing_records() const {
    if (!std::filesystem::exists(log_path_)) {
        return 0;
    }

    std::ifstream input(
        log_path_,
        std::ios::binary
    );

    if (!input) {
        throw std::runtime_error(
            "Failed to inspect log file"
        );
    }

    std::size_t count = 0;

    while (true) {
        std::uint32_t length = 0;

        input.read(
            reinterpret_cast<char*>(&length),
            sizeof(length)
        );

        if (!input) {
            break;
        }

        input.seekg(
            static_cast<std::streamoff>(length),
            std::ios::cur
        );

        if (!input) {
            break;
        }

        ++count;
    }

    return count;
}

// Encode the transaction lifecycle information used by the WAL.
std::vector<std::byte> LogManager::encode_transaction_record(
    std::size_t transaction_id,
    LogRecordType type
) {
    std::vector<std::byte> output;

    append_u32(
        output,
        TRANSACTION_LOG_VERSION
    );

    output.push_back(
        static_cast<std::byte>(
            static_cast<std::uint8_t>(
                LogRecordKind::TRANSACTION
            )
        )
    );

    output.push_back(
        static_cast<std::byte>(
            static_cast<std::uint8_t>(type)
        )
    );

    append_u64(
        output,
        static_cast<std::uint64_t>(
            transaction_id
        )
    );

    return output;
}

// Decode transaction lifecycle information from the WAL.
TransactionLogRecord LogManager::decode_transaction_record(
    const std::vector<std::byte>& record
) {
    std::size_t offset = 0;

    const std::uint32_t version =
        read_u32(record, offset);

    if (version != TRANSACTION_LOG_VERSION) {
        throw std::runtime_error(
            "Unsupported transaction log version"
        );
    }

    if (offset >= record.size()) {
        throw std::runtime_error(
            "Invalid transaction log record"
        );
    }

    const auto kind =
        static_cast<LogRecordKind>(
            std::to_integer<unsigned char>(
                record[offset++]
            )
        );

    if (kind != LogRecordKind::TRANSACTION) {
        throw std::runtime_error(
            "Invalid transaction log record kind"
        );
    }

    if (offset >= record.size()) {
        throw std::runtime_error(
            "Invalid transaction log record"
        );
    }

    const auto type =
        static_cast<LogRecordType>(
            std::to_integer<unsigned char>(
                record[offset++]
            )
        );

    if (type != LogRecordType::BEGIN &&
        type != LogRecordType::COMMIT &&
        type != LogRecordType::ABORT) {
        throw std::runtime_error(
            "Invalid transaction log type"
        );
    }

    const std::size_t transaction_id =
        static_cast<std::size_t>(
            read_u64(record, offset)
        );

    return {
        type,
        transaction_id
    };
}

// Encode all information needed to undo a database modification.
std::vector<std::byte> LogManager::encode_undo_record(
    const UndoLogRecord& record
) {
    std::vector<std::byte> output;

    append_u32(
        output,
        UNDO_LOG_VERSION
    );

    output.push_back(
        static_cast<std::byte>(
            static_cast<std::uint8_t>(
                LogRecordKind::UNDO
            )
        )
    );

    output.push_back(
        static_cast<std::byte>(
            static_cast<std::uint8_t>(
                record.type
            )
        )
    );

    append_u64(
        output,
        static_cast<std::uint64_t>(
            record.transaction_id
        )
    );

    append_string(
        output,
        record.table_name
    );

    append_int(
        output,
        record.page_number
    );

    append_u64(
        output,
        static_cast<std::uint64_t>(
            record.slot_number
        )
    );

    append_string(
        output,
        record.field_name
    );

    append_string(
        output,
        record.old_value
    );

    append_string_map(
        output,
        record.old_record
    );

    return output;
}

// Decode the serialized undo information into an in-memory record.
UndoLogRecord LogManager::decode_undo_record(
    const std::vector<std::byte>& record
) {
    std::size_t offset = 0;

    const std::uint32_t version =
        read_u32(record, offset);

    if (version != UNDO_LOG_VERSION) {
        throw std::runtime_error(
            "Unsupported undo log version"
        );
    }

    if (offset >= record.size()) {
        throw std::runtime_error(
            "Invalid undo log record"
        );
    }

    const auto kind =
        static_cast<LogRecordKind>(
            std::to_integer<unsigned char>(
                record[offset++]
            )
        );

    if (kind != LogRecordKind::UNDO) {
        throw std::runtime_error(
            "Invalid undo log record kind"
        );
    }

    if (offset >= record.size()) {
        throw std::runtime_error(
            "Invalid undo log record"
        );
    }

    UndoLogRecord result;

    result.type =
        static_cast<UndoRecordType>(
            std::to_integer<unsigned char>(
                record[offset++]
            )
        );

    if (result.type != UndoRecordType::INSERT &&
        result.type != UndoRecordType::UPDATE &&
        result.type != UndoRecordType::DELETE) {
        throw std::runtime_error(
            "Invalid undo record type"
        );
    }

    result.transaction_id =
        static_cast<std::size_t>(
            read_u64(record, offset)
        );

    result.table_name =
        read_string(record, offset);

    result.page_number =
        read_int(record, offset);

    result.slot_number =
        static_cast<std::size_t>(
            read_u64(record, offset)
        );

    result.field_name =
        read_string(record, offset);

    result.old_value =
        read_string(record, offset);

    result.old_record =
        read_string_map(record, offset);

    return result;
}

} // namespace flashdb