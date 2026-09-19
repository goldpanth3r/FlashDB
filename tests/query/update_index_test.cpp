#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "database.h"
#include "log/log_manager.h"
#include "planner/plan.h"
#include "query/insert_executor.h"
#include "query/update_executor.h"
#include "tx/transaction.h"

namespace flashdb {

TEST(UpdateIndexTest, RollbackRestoresOldIndexEntry) {
    const std::string database_directory =
        "test_update_index_rollback_db";

    // Start this test with a clean database directory.
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

    // Insert the original row.
    Transaction insert_transaction(
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
                "2"
            ),
            Expression(
                ExpressionType::STRING,
                "Bob"
            )
        }
    );

    InsertExecutor insert_executor(
        insert_plan,
        database,
        insert_transaction
    );

    const RecordId rid =
        insert_executor.execute();

    insert_transaction.commit();

    // Create an index on the id column.
    database.index_manager().create_index(
        "student_id_idx",
        "student",
        "id"
    );

    // Add the existing row to the index.
    database.index_manager().insert(
        "student_id_idx",
        2,
        rid
    );

    // Update id from 2 to 3 inside a transaction.
    Transaction update_transaction(
        database,
        log_manager
    );

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

    Plan update_plan(
        "Update",
        "student",
        nullptr,
        condition,
        {
            Expression(
                ExpressionType::IDENTIFIER,
                "id"
            )
        },
        {
            Expression(
                ExpressionType::INTEGER,
                "3"
            )
        }
    );

    UpdateExecutor update_executor(
        update_plan,
        database,
        update_transaction
    );

    EXPECT_EQ(
        update_executor.execute(),
        1
    );

    // Roll back the update.
    update_transaction.rollback();

    // The old index entry must be restored.
    const std::vector<RecordId> old_results =
        database.index_manager().search_all(
            "student_id_idx",
            2
        );

    ASSERT_EQ(
        old_results.size(),
        1
    );

    EXPECT_EQ(
        old_results[0],
        rid
    );

    // The new index entry must be removed.
    const std::vector<RecordId> new_results =
        database.index_manager().search_all(
            "student_id_idx",
            3
        );

    EXPECT_TRUE(
        new_results.empty()
    );

    // The table row must also be restored.
    auto record_file =
        database.open_table("student");

    EXPECT_EQ(
        record_file->get(
            rid,
            "id"
        ),
        "2"
    );
}

TEST(UpdateIndexTest, CommitKeepsNewIndexEntry) {
    const std::string database_directory =
        "test_update_index_commit_db";

    // Start this test with a clean database directory.
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

    // Insert the original row.
    Transaction insert_transaction(
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
                "2"
            ),
            Expression(
                ExpressionType::STRING,
                "Bob"
            )
        }
    );

    InsertExecutor insert_executor(
        insert_plan,
        database,
        insert_transaction
    );

    const RecordId rid =
        insert_executor.execute();

    insert_transaction.commit();

    // Create an index on the id column.
    database.index_manager().create_index(
        "student_id_idx",
        "student",
        "id"
    );

    // Add the existing row to the index.
    database.index_manager().insert(
        "student_id_idx",
        2,
        rid
    );

    // Update id from 2 to 3 inside a transaction.
    Transaction update_transaction(
        database,
        log_manager
    );

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

    Plan update_plan(
        "Update",
        "student",
        nullptr,
        condition,
        {
            Expression(
                ExpressionType::IDENTIFIER,
                "id"
            )
        },
        {
            Expression(
                ExpressionType::INTEGER,
                "3"
            )
        }
    );

    UpdateExecutor update_executor(
        update_plan,
        database,
        update_transaction
    );

    EXPECT_EQ(
        update_executor.execute(),
        1
    );

    // Commit the update.
    update_transaction.commit();

    // The old index entry must be gone.
    const std::vector<RecordId> old_results =
        database.index_manager().search_all(
            "student_id_idx",
            2
        );

    EXPECT_TRUE(
        old_results.empty()
    );

    // The new index entry must remain.
    const std::vector<RecordId> new_results =
        database.index_manager().search_all(
            "student_id_idx",
            3
        );

    ASSERT_EQ(
        new_results.size(),
        1
    );

    EXPECT_EQ(
        new_results[0],
        rid
    );

    // The table row must contain the new value.
    auto record_file =
        database.open_table("student");

    EXPECT_EQ(
        record_file->get(
            rid,
            "id"
        ),
        "3"
    );
}

} // namespace flashdb