#include "log_manager.h"

#include <cstdint>
#include <stdexcept>

namespace flashdb {

LogManager::LogManager(const std::string& database_directory)
    : log_path_(std::filesystem::path(database_directory) / "flashdb.log"),
      record_count_(0) {

    // Make sure the database directory exists.
    std::filesystem::create_directories(database_directory);

    // Open the log in append mode.
    log_file_.open(
        log_path_,
        std::ios::binary |
        std::ios::app
    );

    if (!log_file_) {
        throw std::runtime_error("Unable to open FlashDB log.");
    }
}

std::size_t LogManager::append(
    const std::vector<std::byte>& record) {

    /*
     * Store the record length before the record itself.
     *
     * Log format:
     *
     * [4-byte length][record bytes]
     */
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
        throw std::runtime_error("Failed to append log record.");
    }

    // The first record gets LSN 0, the next gets LSN 1, etc.
    return record_count_++;
}

void LogManager::flush() {
    log_file_.flush();

    if (!log_file_) {
        throw std::runtime_error("Failed to flush FlashDB log.");
    }
}

std::size_t LogManager::size() const {
    return record_count_;
}

} // namespace flashdb