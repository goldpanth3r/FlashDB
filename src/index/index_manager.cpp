#include "index_manager.h"

#include <fstream>
#include <stdexcept>

#include "database.h"
#include "record/record_file.h"

namespace flashdb {

// Store the database directory used for persistent index metadata.
IndexManager::IndexManager(
    const std::string& database_directory)
    : database_directory_(database_directory) {
}

// Load index definitions from the database directory.
void IndexManager::load() {

    if (database_directory_.empty()) {
        return;
    }

    const std::string path =
        database_directory_ + "/flashdb.indexes";

    std::ifstream input(path);

    if (!input.is_open()) {
        return;
    }

    std::string name;
    std::string table_name;
    std::string column_name;

    while (std::getline(input, name)) {

        if (!std::getline(input, table_name)) {
            break;
        }

        if (!std::getline(input, column_name)) {
            break;
        }

        if (name.empty() ||
            table_name.empty() ||
            column_name.empty()) {
            continue;
        }

        indexes_.try_emplace(name);

        metadata_[name] = IndexMetadata{
            name,
            table_name,
            column_name
        };
    }
}

// Save all index definitions to disk.
void IndexManager::save() const {

    if (database_directory_.empty()) {
        return;
    }

    const std::string path =
        database_directory_ + "/flashdb.indexes";

    std::ofstream output(
        path,
        std::ios::trunc
    );

    if (!output.is_open()) {
        throw std::runtime_error(
            "IndexManager: failed to save index metadata"
        );
    }

    for (const auto& [name, metadata] :
         metadata_) {

        output << metadata.name << '\n';
        output << metadata.table_name << '\n';
        output << metadata.column_name << '\n';
    }
}

// Rebuild every loaded index by scanning its table.
void IndexManager::rebuild_indexes(
    Database& database) {

    for (const auto& [name, metadata] :
         metadata_) {

        std::unique_ptr<RecordFile> record_file =
            database.open_table(
                metadata.table_name
            );

        for (const RecordId& rid :
             record_file->scan()) {

            const std::string value =
                record_file->get(
                    rid,
                    metadata.column_name
                );

            int key;

            try {
                key = std::stoi(value);
            }
            catch (const std::exception&) {
                throw std::invalid_argument(
                    "IndexManager: indexed column must contain integers"
                );
            }

            indexes_.at(name).insert(
                key,
                rid
            );
        }
    }
}

// Create an index without table metadata.
void IndexManager::create_index(
    const std::string& name) {

    if (name.empty()) {
        throw std::invalid_argument(
            "IndexManager: index name cannot be empty"
        );
    }

    indexes_.try_emplace(name);
}

// Create an index and persist its table metadata.
void IndexManager::create_index(
    const std::string& name,
    const std::string& table_name,
    const std::string& column_name) {

    if (name.empty()) {
        throw std::invalid_argument(
            "IndexManager: index name cannot be empty"
        );
    }

    if (table_name.empty()) {
        throw std::invalid_argument(
            "IndexManager: table name cannot be empty"
        );
    }

    if (column_name.empty()) {
        throw std::invalid_argument(
            "IndexManager: column name cannot be empty"
        );
    }

    if (indexes_.contains(name)) {
        throw std::invalid_argument(
            "IndexManager: index already exists"
        );
    }

    indexes_.emplace(
        name,
        BPlusTree{}
    );

    metadata_.emplace(
        name,
        IndexMetadata{
            name,
            table_name,
            column_name
        }
    );

    if (!database_directory_.empty()) {
        save();
    }
}

// Check whether an index exists.
bool IndexManager::has_index(
    const std::string& name) const {

    return indexes_.contains(name);
}

// Return the B+ Tree belonging to an index.
BPlusTree& IndexManager::index(
    const std::string& name) {

    auto it = indexes_.find(name);

    if (it == indexes_.end()) {
        throw std::runtime_error(
            "IndexManager: index does not exist"
        );
    }

    return it->second;
}

// Return metadata belonging to an index.
const IndexMetadata& IndexManager::metadata(
    const std::string& name) const {

    auto it = metadata_.find(name);

    if (it == metadata_.end()) {
        throw std::runtime_error(
            "IndexManager: index metadata does not exist"
        );
    }

    return it->second;
}

// Find all indexes associated with a table.
std::vector<std::string>
IndexManager::indexes_for_table(
    const std::string& table_name) const {

    std::vector<std::string> result;

    for (const auto& [name, metadata] :
         metadata_) {

        if (metadata.table_name == table_name) {
            result.push_back(name);
        }
    }

    return result;
}

// Add a key and record ID to an index.
void IndexManager::insert(
    const std::string& name,
    int key,
    const RecordId& rid) {

    index(name).insert(
        key,
        rid
    );
}

// Search an index for one matching record.
bool IndexManager::search(
    const std::string& name,
    int key,
    RecordId& rid) const {

    auto it = indexes_.find(name);

    if (it == indexes_.end()) {
        throw std::runtime_error(
            "IndexManager: index does not exist"
        );
    }

    return it->second.search(
        key,
        rid
    );
}

// Return all record IDs matching the given key.
std::vector<RecordId>
IndexManager::search_all(
    const std::string& name,
    int key) const {

    auto it = indexes_.find(name);

    if (it == indexes_.end()) {
        throw std::runtime_error(
            "IndexManager: index does not exist"
        );
    }

    return it->second.range_scan(
        key,
        key
    );
}

// Remove one specific record from an index.
bool IndexManager::remove(
    const std::string& name,
    int key,
    const RecordId& rid) {

    return index(name).remove(
        key,
        rid
    );
}

} // namespace flashdb