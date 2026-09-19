#ifndef FLASHDB_INDEX_INDEX_MANAGER_H
#define FLASHDB_INDEX_INDEX_MANAGER_H

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "b_plus_tree.h"
#include "record/rid.h"

namespace flashdb {

class Database;

struct IndexMetadata {
    std::string name;
    std::string table_name;
    std::string column_name;
};

// Owns B+ Tree indexes and persists their metadata.
class IndexManager {
public:
    explicit IndexManager(
        const std::string& database_directory = ""
    );

    // Load index definitions saved on disk.
    void load();

    // Save index definitions to disk.
    void save() const;

    // Rebuild loaded indexes from table records.
    void rebuild_indexes(
        Database& database
    );

    // Create an index without table metadata.
    void create_index(
        const std::string& name
    );

    // Create an index with table and column metadata.
    void create_index(
        const std::string& name,
        const std::string& table_name,
        const std::string& column_name
    );

    // Check whether an index exists.
    bool has_index(
        const std::string& name
    ) const;

    // Return the B+ Tree for an index.
    BPlusTree& index(
        const std::string& name
    );

    // Return metadata for an index.
    const IndexMetadata& metadata(
        const std::string& name
    ) const;

    // Return all indexes belonging to a table.
    std::vector<std::string> indexes_for_table(
        const std::string& table_name
    ) const;

    // Add a key and record ID to an index.
    void insert(
        const std::string& name,
        int key,
        const RecordId& rid
    );

    // Search an index for one matching record.
    bool search(
        const std::string& name,
        int key,
        RecordId& rid
    ) const;

    // Return all records matching a key.
    std::vector<RecordId> search_all(
        const std::string& name,
        int key
    ) const;

    // Remove one record from an index.
    bool remove(
        const std::string& name,
        int key,
        const RecordId& rid
    );

private:
    std::string database_directory_;

    std::unordered_map<
        std::string,
        BPlusTree
    > indexes_;

    std::unordered_map<
        std::string,
        IndexMetadata
    > metadata_;
};

} // namespace flashdb

#endif