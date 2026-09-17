#include "log_manager.h"

#include <cstdint>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace flashdb {

/**
 * Create or open the FlashDB log.
 *
 * @param database_directory Directory containing database files.
 */
LogManager::LogManager(
    const std::string& database_directory)
    : log_path_(
          std::filesystem::path(database_directory)
          / "flashdb.log"),
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
            "LogManager: unable to open log"
        );
    }
}

/**
 * count_existing_records scans the log file.
 *
 * @return Number of complete records already stored.
 */
std::size_t LogManager::count_existing_records() const {
    if (!std::filesystem::exists(log_path_)) {
        return 0;
    }

    std::ifstream file(
        log_path_,
        std::ios::binary
    );

    if (!file) {
        throw std::runtime_error(
            "LogManager: unable to read existing log"
        );
    }

    std::size_t count = 0;

    while (true) {
        std::uint32_t length = 0;

        file.read(
            reinterpret_cast<char*>(&length),
            sizeof(length)
        );

        if (file.eof()) {
            break;
        }

        if (!file) {
            throw std::runtime_error(
                "LogManager: corrupted log header"
            );
        }

        file.seekg(
            static_cast<std::streamoff>(length),
            std::ios::cur
        );

        if (!file) {
            throw std::runtime_error(
                "LogManager: corrupted log record"
            );
        }

        ++count;
    }

    return count;
}

/**
 * append writes one record to the end of the log.
 *
 * @param record Bytes representing the record.
 * @return LSN assigned to the record.
 */
std::size_t LogManager::append(
    const std::vector<std::byte>& record) {

    if (record.size() >
        std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error(
            "LogManager: record is too large"
        );
    }

    const std::uint32_t length =
        static_cast<std::uint32_t>(record.size());

    const std::size_t lsn = record_count_;

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
            "LogManager: failed to append record"
        );
    }

    ++record_count_;

    return lsn;
}

/**
 * read loads one record from the log.
 *
 * @param lsn Log sequence number.
 * @return Stored record bytes.
 */
std::vector<std::byte> LogManager::read(
    std::size_t lsn) const {

    std::ifstream file(
        log_path_,
        std::ios::binary
    );

    if (!file) {
        throw std::runtime_error(
            "LogManager: unable to open log for reading"
        );
    }

    for (std::size_t current = 0;
         current <= lsn;
         ++current) {

        std::uint32_t length = 0;

        file.read(
            reinterpret_cast<char*>(&length),
            sizeof(length)
        );

        if (!file) {
            throw std::out_of_range(
                "LogManager: invalid LSN"
            );
        }

        if (current == lsn) {
            std::vector<std::byte> record(length);

            if (length > 0) {
                file.read(
                    reinterpret_cast<char*>(record.data()),
                    static_cast<std::streamsize>(length)
                );

                if (!file) {
                    throw std::runtime_error(
                        "LogManager: incomplete log record"
                    );
                }
            }

            return record;
        }

        file.seekg(
            static_cast<std::streamoff>(length),
            std::ios::cur
        );

        if (!file) {
            throw std::runtime_error(
                "LogManager: corrupted log"
            );
        }
    }

    throw std::out_of_range(
        "LogManager: invalid LSN"
    );
}

/**
 * flush forces pending log data to the file.
 */
void LogManager::flush() {
    log_file_.flush();

    if (!log_file_) {
        throw std::runtime_error(
            "LogManager: flush failed"
        );
    }
}

/**
 * size returns the number of log records.
 *
 * @return Number of records.
 */
std::size_t LogManager::size() const {
    return record_count_;
}

} // namespace flashdb