#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "database.h"
#include "log/log_manager.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "planner/planner.h"
#include "query/create_index_executor.h"
#include "query/insert_executor.h"
#include "query/query_executor.h"
#include "tx/transaction.h"

namespace flashdb {

TEST(CreateIndexTest, SqlCreateIndexBuildsIndex) {
    const std::string database_directory =
        "test_create_index_sql_db";

    std::filesystem::remove_all(
        database_directory
    );

    Database database(database_directory);
    LogManager log_manager(database_directory);

    Schema schema;
    schema.add_int_field("id");
    schema.add_string_field("name", 50);

    database.catalog().create_table(
        "student",
        schema
    );

    Transaction transaction(
        database,
        log_manager
    );

    Plan insert_plan(
        "Insert",
        "student",
        nullptr,
        std::nullopt,
        {},
        {
            Expression(
                ExpressionType::INTEGER,
                "10"
            ),
            Expression(
                ExpressionType::STRING,
                "Alice"
            )
        }
    );

    InsertExecutor insert_executor(
        insert_plan,
        database,
        transaction
    );

    const RecordId rid =
        insert_executor.execute();

    transaction.commit();

    Lexer lexer(
        "CREATE INDEX student_id_idx "
        "ON student(id);"
    );

    const auto tokens =
        lexer.tokenize();

    Parser parser(tokens);

    const auto statement =
        parser.parse();

    Planner planner;

    const std::unique_ptr<Plan> plan =
        planner.create_plan(statement);

    ASSERT_NE(
        plan,
        nullptr
    );

    EXPECT_EQ(
        plan->get_name(),
        "CreateIndex"
    );

    CreateIndexExecutor executor(
        *plan,
        database
    );

    executor.execute();

    ASSERT_TRUE(
        database.index_manager().has_index(
            "student_id_idx"
        )
    );

    const std::vector<RecordId> results =
        database.index_manager().search_all(
            "student_id_idx",
            10
        );

    ASSERT_EQ(
        results.size(),
        1
    );

    EXPECT_EQ(
        results[0],
        rid
    );
}

TEST(CreateIndexTest, SqlCreateIndexSupportsIndexedSelect) {
    const std::string database_directory =
        "test_create_index_select_db";

    std::filesystem::remove_all(
        database_directory
    );

    Database database(database_directory);
    LogManager log_manager(database_directory);

    Schema schema;
    schema.add_int_field("id");
    schema.add_string_field("name", 50);

    database.catalog().create_table(
        "student",
        schema
    );

    Transaction transaction(
        database,
        log_manager
    );

    Plan insert_plan(
        "Insert",
        "student",
        nullptr,
        std::nullopt,
        {},
        {
            Expression(
                ExpressionType::INTEGER,
                "10"
            ),
            Expression(
                ExpressionType::STRING,
                "Alice"
            )
        }
    );

    InsertExecutor insert_executor(
        insert_plan,
        database,
        transaction
    );

    insert_executor.execute();

    transaction.commit();

    // Create the index through SQL.
    Lexer index_lexer(
        "CREATE INDEX student_id_idx "
        "ON student(id);"
    );

    const auto index_tokens =
        index_lexer.tokenize();

    Parser index_parser(index_tokens);

    const auto index_statement =
        index_parser.parse();

    Planner planner;

    const std::unique_ptr<Plan> index_plan =
        planner.create_plan(
            index_statement
        );

    CreateIndexExecutor index_executor(
        *index_plan,
        database
    );

    index_executor.execute();

    // Build SELECT through the complete SQL pipeline.
    Lexer select_lexer(
        "SELECT name "
        "FROM student "
        "WHERE id = 10;"
    );

    const auto select_tokens =
        select_lexer.tokenize();

    Parser select_parser(select_tokens);

    const auto select_statement =
        select_parser.parse();

    const std::unique_ptr<Plan> select_plan =
        planner.create_plan(
            select_statement
        );

    QueryExecutor query_executor(
        *select_plan,
        database
    );

    const auto results =
        query_executor.execute();

    ASSERT_EQ(
        results.size(),
        1
    );

    ASSERT_EQ(
        results[0].size(),
        1
    );

    EXPECT_EQ(
        results[0][0],
        "Alice"
    );
}

} // namespace flashdb