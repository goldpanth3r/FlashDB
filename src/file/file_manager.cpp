#include "file_manager.h"

#include <fstream>
#include <stdexcept>

namespace flashdb {

FileManager::FileManager(const std::string& database_directory)
    : db_directory_(database_directory) {

    std::filesystem::create_directories(db_directory_);
}

std::filesystem::path FileManager::file_path(
    const std::string& filename) const {
    return db_directory_ / filename;
}

void FileManager::read(const BlockId& block, Page& page) {
    std::fstream file(file_path(block.filename()),
                      std::ios::binary | std::ios::in);

    if (!file)
        throw std::runtime_error("Cannot open file for reading.");

    file.seekg(
        static_cast<std::streamoff>(block.number()) * Page::PAGE_SIZE);

    file.read(reinterpret_cast<char*>(page.data().data()),
              Page::PAGE_SIZE);
}

void FileManager::write(const BlockId& block, const Page& page) {
    std::fstream file(file_path(block.filename()),
                      std::ios::binary |
                      std::ios::in |
                      std::ios::out);

    if (!file) {
        file.open(file_path(block.filename()),
                  std::ios::binary |
                  std::ios::out);
        file.close();

        file.open(file_path(block.filename()),
                  std::ios::binary |
                  std::ios::in |
                  std::ios::out);
    }

    file.seekp(
        static_cast<std::streamoff>(block.number()) * Page::PAGE_SIZE);

    file.write(reinterpret_cast<const char*>(page.data().data()),
               Page::PAGE_SIZE);

    file.flush();
}

BlockId FileManager::append(const std::string& filename) {

    int new_block = length(filename);

    std::fstream file(file_path(filename),
                      std::ios::binary |
                      std::ios::app);

    std::vector<std::byte> empty(Page::PAGE_SIZE);

    file.write(reinterpret_cast<const char*>(empty.data()),
               Page::PAGE_SIZE);

    file.close();

    return BlockId(filename, new_block);
}

int FileManager::length(const std::string& filename) {

    auto path = file_path(filename);

    if (!std::filesystem::exists(path))
        return 0;

    auto bytes = std::filesystem::file_size(path);

    return static_cast<int>(bytes / Page::PAGE_SIZE);
}

} // namespace flashdb