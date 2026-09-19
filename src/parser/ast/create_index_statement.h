#pragma once

#include <string>

namespace flashdb {

class CreateIndexStatement {
public:
    // Store the index name, table name, and indexed column.
    CreateIndexStatement(
        std::string index_name,
        std::string table_name,
        std::string column_name
    );

    // Return the index name.
    const std::string& index_name() const;

    // Return the table containing the indexed column.
    const std::string& table_name() const;

    // Return the column being indexed.
    const std::string& column_name() const;

private:
    std::string index_name_;
    std::string table_name_;
    std::string column_name_;
};

} // namespace flashdb
