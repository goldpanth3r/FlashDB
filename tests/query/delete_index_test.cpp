#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "database.h"
#include "log/log_manager.h"
#include "planner/plan.h"
#include "query/delete_executor.h"
#include "query/insert_executor.h"
#include "tx/transaction.h"

namespace flashdb {

namespace {

void create_student_table(Database& database) {
    Schema schema;

    schema.add_int_field("id");
    schema.add_string_field("name", 50);

    database.catalog().create_table(
        "student",
        schema
    );
}

RecordId insert_student(
    Database& database,
    LogManager& log_manager) {

    Transaction transaction(
        database,
        log_manager
    );

    Plan plan(
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

    InsertExecutor executor(
        plan,
        database,
        transaction
    );

    const RecordId rid =
        executor.execute();

    transaction.commit();

    return rid;
}

void create_student_index(
    Database& database,
    const RecordId& rid) {

    database.index_manager().create_index(
        "student_id_idx",
        "student",
        "id"
    );

    database.index_manager().insert(
        "student_id_idx",
        2,
        rid
    );
}

Plan create_delete_plan() {
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

    return Plan(
        "Delete",
        "student",
        nullptr,
        condition
    );
}

} // namespace

TEST(DeleteIndexTest, RollbackRestoresDeletedRowAndIndexEntry) {
    const std::string database_directory =
        "test_delete_index_rollback_db";

    std::filesystem::remove_all(
        database_directory
    );

    Database database(database_directory);
    LogManager log_manager(database_directory);

    create_student_table(
        database
    );

    const RecordId rid =
        insert_student(
            database,
            log_manager
        );

    create_student_index(
        database,
        rid
    );

    Transaction delete_transaction(
        database,
        log_manager
    );

    Plan delete_plan =
        create_delete_plan();

    DeleteExecutor delete_executor(
        delete_plan,
        database,
        delete_transaction
    );

    EXPECT_EQ(
        delete_executor.execute(),
        1
    );

    // Roll back the deletion.
    delete_transaction.rollback();

    // The row must exist again.
    auto record_file =
        database.open_table("student");

    EXPECT_EQ(
        record_file->get(
            rid,
            "id"
        ),
        "2"
    );

    // The old index entry must also exist again.
    const std::vector<RecordId> results =
        database.index_manager().search_all(
            "student_id_idx",
            2
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

TEST(DeleteIndexTest, CommitRemovesDeletedRowAndIndexEntry) {
    const std::string database_directory =
        "test_delete_index_commit_db";

    std::filesystem::remove_all(
        database_directory
    );

    Database database(database_directory);
    LogManager log_manager(database_directory);

    create_student_table(
        database
    );

    const RecordId rid =
        insert_student(
            database,
            log_manager
        );

    create_student_index(
        database,
        rid
    );

    Transaction delete_transaction(
        database,
        log_manager
    );

    Plan delete_plan =
        create_delete_plan();

    DeleteExecutor delete_executor(
        delete_plan,
        database,
        delete_transaction
    );

    EXPECT_EQ(
        delete_executor.execute(),
        1
    );

    // Commit the deletion.
    delete_transaction.commit();

    // The index entry must be gone.
    const std::vector<RecordId> results =
        database.index_manager().search_all(
            "student_id_idx",
            2
        );

    EXPECT_TRUE(
        results.empty()
    );

    // The deleted row must no longer be visible.
    auto record_file =
        database.open_table("student");

    EXPECT_THROW(
        record_file->get(
            rid,
            "id"
        ),
        std::exception
    );
}

} // namespace flashdb