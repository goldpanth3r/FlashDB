#include <filesystem>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "database.h"
#include "planner/plan.h"
#include "query/insert_executor.h"
#include "query/query_executor.h"

namespace flashdb {

TEST(IndexedSelectTest, UsesIndexForEqualityCondition) {

    const std::string database_directory =
        "test_indexed_select_db";

    // Remove persistent data from previous test runs.
    std::filesystem::remove_all(
        database_directory
    );

    Database database(
        database_directory
    );

    Schema schema;

    schema.add_int_field("id");
    schema.add_string_field("name", 50);

    database.catalog().create_table(
        "student",
        schema
    );

    database.index_manager().create_index(
        "student_id_idx",
        "student",
        "id"
    );

    // Insert the first student.
    Plan insert_plan_1(
        "Insert",
        "student",
        nullptr,
        std::nullopt,
        {},
        {
            Expression(
                ExpressionType::INTEGER,
                "1"
            ),
            Expression(
                ExpressionType::STRING,
                "Alice"
            )
        }
    );

    InsertExecutor insert_1(
        insert_plan_1,
        database
    );

    insert_1.execute();

    // Insert the second student.
    Plan insert_plan_2(
        "Insert",
        "student",
        nullptr,
        std::nullopt,
        {},
        {
            Expression(
                ExpressionType::INTEGER,
                "2"
            ),
            Expression(
                ExpressionType::STRING,
                "Bob"
            )
        }
    );

    InsertExecutor insert_2(
        insert_plan_2,
        database
    );

    insert_2.execute();

    // Build WHERE id = 2.
    Condition condition(
        Expression(
            ExpressionType::IDENTIFIER,
            "id"
        ),
        "=",
        Expression(
            ExpressionType::INTEGER,
            "2"
        )
    );

    auto table_scan =
        std::make_unique<Plan>(
            "TableScan",
            "student"
        );

    auto filter =
        std::make_unique<Plan>(
            "Filter",
            "student",
            std::move(table_scan),
            condition
        );

    Plan project(
        "Project",
        "student",
        std::move(filter),
        std::nullopt,
        {
            Expression(
                ExpressionType::IDENTIFIER,
                "name"
            )
        }
    );

    QueryExecutor executor(
        project,
        database
    );

    const auto results =
        executor.execute();

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
        "Bob"
    );

    // Remove persistent test data.
    database.buffer_manager().flush_all();

    std::filesystem::remove_all(
        database_directory
    );
}

} // namespace flashdb