#include "file_manager.h"

#include <fstream>
#include <stdexcept>
#include <vector>

namespace flashdb {

/**
 * FileManager creates a database file manager.
 *
 * @param database_directory Directory where database files are stored.
 */
FileManager::FileManager(const std::string& database_directory)
    : db_directory_(database_directory) {

    std::filesystem::create_directories(db_directory_);

    if (!std::filesystem::exists(db_directory_)) {
        throw std::runtime_error(
            "FileManager: unable to create database directory"
        );
    }
}

/**
 * file_path builds the full path for a database file.
 *
 * @param filename Database file name.
 * @return Full path to the database file.
 */
std::filesystem::path FileManager::file_path(
    const std::string& filename) const {

    if (filename.empty()) {
        throw std::invalid_argument(
            "FileManager: filename cannot be empty"
        );
    }

    return db_directory_ / filename;
}

/**
 * read loads one complete database page from disk.
 *
 * @param block Block to read.
 * @param page Page where the data will be stored.
 */
void FileManager::read(
    const BlockId& block,
    Page& page) {

    if (block.number() < 0) {
        throw std::invalid_argument(
            "FileManager::read: invalid block number"
        );
    }

    std::ifstream file(
        file_path(block.filename()),
        std::ios::binary
    );

    if (!file) {
        throw std::runtime_error(
            "FileManager::read: unable to open file"
        );
    }

    const auto offset =
        static_cast<std::streamoff>(block.number())
        * static_cast<std::streamoff>(Page::PAGE_SIZE);

    file.seekg(offset);

    if (!file) {
        throw std::runtime_error(
            "FileManager::read: seek failed"
        );
    }

    file.read(
        reinterpret_cast<char*>(page.data().data()),
        static_cast<std::streamsize>(Page::PAGE_SIZE)
    );

    if (file.gcount() !=
        static_cast<std::streamsize>(Page::PAGE_SIZE)) {

        throw std::runtime_error(
            "FileManager::read: incomplete page read"
        );
    }
}

/**
 * write stores one complete database page on disk.
 *
 * @param block Block where the page will be written.
 * @param page Page containing the data.
 */
void FileManager::write(
    const BlockId& block,
    const Page& page) {

    if (block.number() < 0) {
        throw std::invalid_argument(
            "FileManager::write: invalid block number"
        );
    }

    const auto path = file_path(block.filename());

    std::fstream file(
        path,
        std::ios::binary |
        std::ios::in |
        std::ios::out
    );

    if (!file) {
        file.clear();

        file.open(
            path,
            std::ios::binary |
            std::ios::out
        );

        if (!file) {
            throw std::runtime_error(
                "FileManager::write: unable to create file"
            );
        }

        file.close();

        file.open(
            path,
            std::ios::binary |
            std::ios::in |
            std::ios::out
        );
    }

    if (!file) {
        throw std::runtime_error(
            "FileManager::write: unable to open file"
        );
    }

    const auto offset =
        static_cast<std::streamoff>(block.number())
        * static_cast<std::streamoff>(Page::PAGE_SIZE);

    file.seekp(offset);

    if (!file) {
        throw std::runtime_error(
            "FileManager::write: seek failed"
        );
    }

    file.write(
        reinterpret_cast<const char*>(page.data().data()),
        static_cast<std::streamsize>(Page::PAGE_SIZE)
    );

    if (!file) {
        throw std::runtime_error(
            "FileManager::write: page write failed"
        );
    }

    file.flush();

    if (!file) {
        throw std::runtime_error(
            "FileManager::write: flush failed"
        );
    }
}

/**
 * append creates one empty page at the end of a database file.
 *
 * @param filename Database file name.
 * @return BlockId identifying the newly created page.
 */
BlockId FileManager::append(
    const std::string& filename) {

    const int new_block = length(filename);

    std::ofstream file(
        file_path(filename),
        std::ios::binary |
        std::ios::app
    );

    if (!file) {
        throw std::runtime_error(
            "FileManager::append: unable to open file"
        );
    }

    const std::vector<std::byte> empty_page(
        Page::PAGE_SIZE,
        std::byte{0}
    );

    file.write(
        reinterpret_cast<const char*>(empty_page.data()),
        static_cast<std::streamsize>(empty_page.size())
    );

    if (!file) {
        throw std::runtime_error(
            "FileManager::append: page write failed"
        );
    }

    file.flush();

    if (!file) {
        throw std::runtime_error(
            "FileManager::append: flush failed"
        );
    }

    return BlockId(filename, new_block);
}

/**
 * length returns the number of complete pages in a database file.
 *
 * @param filename Database file name.
 * @return Number of complete pages.
 */
int FileManager::length(
    const std::string& filename) {

    const auto path = file_path(filename);

    if (!std::filesystem::exists(path)) {
        return 0;
    }

    const auto bytes =
        std::filesystem::file_size(path);

    if (bytes % Page::PAGE_SIZE != 0) {
        throw std::runtime_error(
            "FileManager::length: file contains a partial page"
        );
    }

    return static_cast<int>(
        bytes / Page::PAGE_SIZE
    );
}

} // namespace flashdb