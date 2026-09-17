#ifndef FLASHDB_LOG_LOG_MANAGER_H
#define FLASHDB_LOG_LOG_MANAGER_H

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace flashdb {

/**
 * LogManager stores database log records on disk.
 *
 * The log is append-only.
 * New log records are written after existing records.
 *
 * Later, the transaction and recovery systems will use this
 * log to recover the database after a crash.
 */
class LogManager {
public:
    /**
     * Create or open the database log.
     *
     * @param database_directory Directory containing FlashDB data.
     */
    explicit LogManager(const std::string& database_directory);

    /**
     * Append a record to the log.
     *
     * @param record Bytes representing one log record.
     * @return Log sequence number (LSN) assigned to the record.
     */
    std::size_t append(const std::vector<std::byte>& record);

    /**
     * Flush all pending log data to disk.
     */
    void flush();

    /**
     * Return the number of log records currently stored.
     */
    std::size_t size() const;

private:
    std::filesystem::path log_path_;
    std::ofstream log_file_;
    std::size_t record_count_;
};

} // namespace flashdb

#endif