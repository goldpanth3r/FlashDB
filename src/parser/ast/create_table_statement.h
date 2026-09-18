#ifndef FLASHDB_PARSER_AST_CREATE_TABLE_STATEMENT_H
#define FLASHDB_PARSER_AST_CREATE_TABLE_STATEMENT_H

#include <string>
#include <vector>

namespace flashdb {

/**
 * ColumnDefinition represents one column in a table.
 *
 * Example:
 * id INT
 * name VARCHAR
 */
struct ColumnDefinition {
    std::string name;
    std::string type;
    int length;
};

/**
 * CreateTableStatement represents a SQL CREATE TABLE statement.
 *
 * Example:
 * CREATE TABLE student(
 *     id INT,
 *     name VARCHAR(100)
 * );
 */
class CreateTableStatement {
public:
    /**
     * Create a CREATE TABLE statement.
     *
     * The arguments contain the table name and column definitions.
     */
    CreateTableStatement(
        std::string table_name,
        std::vector<ColumnDefinition> columns
    );

    /**
     * Return the table name.
     */
    const std::string& table_name() const;

    /**
     * Return the column definitions.
     */
    const std::vector<ColumnDefinition>& columns() const;

private:
    std::string table_name_;
    std::vector<ColumnDefinition> columns_;
};

} // namespace flashdb

#endif