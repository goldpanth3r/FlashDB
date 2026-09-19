#include <filesystem>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "database.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "planner/planner.h"
#include "query/create_index_executor.h"
#include "query/create_table_executor.h"
#include "query/insert_executor.h"
#include "query/query_executor.h"

namespace flashdb {
namespace {

const std::string DATABASE_DIRECTORY =
    "test_index_persistence_db";

// Convert SQL text into an executable plan.
std::unique_ptr<Plan> create_plan(
    const std::string& sql) {

    Lexer lexer(sql);

    const auto tokens =
        lexer.tokenize();

    Parser parser(tokens);

    const auto statement =
        parser.parse();

    Planner planner;

    return planner.create_plan(
        statement
    );
}

// Execute a SELECT statement through QueryExecutor.
std::vector<std::vector<std::string>> execute_select(
    Database& database,
    const std::string& sql) {

    std::unique_ptr<Plan> plan =
        create_plan(sql);

    QueryExecutor executor(
        *plan,
        database
    );

    return executor.execute();
}

} // namespace

TEST(
    IndexPersistenceTest,
    IndexSurvivesDatabaseRestart) {

    // Remove persistent data from previous runs.
    std::filesystem::remove_all(
        DATABASE_DIRECTORY
    );

    // Create the table, rows, and index.
    {
        Database database(
            DATABASE_DIRECTORY
        );

        std::unique_ptr<Plan> create_table_plan =
            create_plan(
                "CREATE TABLE student("
                "id INT, "
                "name VARCHAR(50));"
            );

        CreateTableExecutor create_table(
            *create_table_plan,
            database
        );

        create_table.execute();

        std::unique_ptr<Plan> insert_plan_1 =
            create_plan(
                "INSERT INTO student "
                "VALUES (1, 'Alice');"
            );

        InsertExecutor insert_1(
            *insert_plan_1,
            database
        );

        insert_1.execute();

        std::unique_ptr<Plan> insert_plan_2 =
            create_plan(
                "INSERT INTO student "
                "VALUES (2, 'Bob');"
            );

        InsertExecutor insert_2(
            *insert_plan_2,
            database
        );

        insert_2.execute();

        std::unique_ptr<Plan> create_index_plan =
            create_plan(
                "CREATE INDEX student_id_idx "
                "ON student(id);"
            );

        CreateIndexExecutor create_index(
            *create_index_plan,
            database
        );

        create_index.execute();

        // Confirm the index works before restart.
        EXPECT_TRUE(
            database.index_manager().has_index(
                "student_id_idx"
            )
        );

        const auto records =
            database.index_manager().search_all(
                "student_id_idx",
                1
            );

        ASSERT_EQ(
            records.size(),
            1u
        );
    }

    // Reopen the same database directory.
    {
        Database database(
            DATABASE_DIRECTORY
        );

        // Confirm index metadata was restored.
        EXPECT_TRUE(
            database.index_manager().has_index(
                "student_id_idx"
            )
        );

        // Confirm the B+ Tree was rebuilt.
        const auto records =
            database.index_manager().search_all(
                "student_id_idx",
                1
            );

        ASSERT_EQ(
            records.size(),
            1u
        );

        // Confirm the rebuilt RecordId points to Alice.
        std::unique_ptr<RecordFile> table =
            database.open_table(
                "student"
            );

        EXPECT_EQ(
            table->get(
                records[0],
                "name"
            ),
            "Alice"
        );

        // Confirm indexed SELECT works after restart.
        const auto results =
            execute_select(
                database,
                "SELECT name "
                "FROM student "
                "WHERE id = 1;"
            );

        ASSERT_EQ(
            results.size(),
            1u
        );

        ASSERT_EQ(
            results[0].size(),
            1u
        );

        EXPECT_EQ(
            results[0][0],
            "Alice"
        );
    }

    // Remove persistent test data.
    std::filesystem::remove_all(
        DATABASE_DIRECTORY
    );
}

} // namespace flashdb