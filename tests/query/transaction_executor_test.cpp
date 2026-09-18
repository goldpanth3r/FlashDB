#include <filesystem>

#include <gtest/gtest.h>

#include "database.h"
#include "log/log_manager.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "planner/planner.h"
#include "query/delete_executor.h"
#include "query/insert_executor.h"
#include "query/update_executor.h"
#include "tx/transaction.h"

namespace flashdb {

class TransactionExecutorTest : public ::testing::Test {
protected:
    std::filesystem::path database_directory;

    void SetUp() override {
        database_directory =
            std::filesystem::temp_directory_path() /
            "flashdb_transaction_executor_test";

        std::filesystem::remove_all(database_directory);
    }

    void TearDown() override {
        std::filesystem::remove_all(database_directory);
    }

    void create_student_table(Database& database) {
        Schema schema;
        schema.add_int_field("id");
        schema.add_string_field("name", 50);

        database.catalog().create_table(
            "student",
            schema
        );

        database.catalog().save();

        database.file_manager().append(
            "student.tbl"
        );
    }

    std::unique_ptr<Plan> create_plan(
        const std::string& sql) {

        Lexer lexer(sql);

        const auto tokens = lexer.tokenize();
        Parser parser(tokens);

        const auto statement = parser.parse();

        Planner planner;

        return planner.create_plan(statement);
    }
};

// Verify that INSERT executed through a transaction can be rolled back.
TEST_F(
    TransactionExecutorTest,
    RollbackInsertExecutor) {

    Database database(
        database_directory.string()
    );

    create_student_table(database);

    LogManager log_manager(
        database_directory.string()
    );

    Transaction transaction(
        database,
        log_manager
    );

    auto plan =
        create_plan(
            "INSERT INTO student VALUES (1, 'Alice');"
        );

    InsertExecutor executor(
        *plan,
        database,
        transaction
    );

    const RecordId rid =
        executor.execute();

    EXPECT_EQ(
        database.open_table("student")->get(
            rid,
            "name"
        ),
        "Alice"
    );

    transaction.rollback();

    EXPECT_TRUE(
        database.open_table("student")->scan().empty()
    );
}

// Verify that UPDATE executed through a transaction can be rolled back.
TEST_F(
    TransactionExecutorTest,
    RollbackUpdateExecutor) {

    Database database(
        database_directory.string()
    );

    create_student_table(database);

    LogManager log_manager(
        database_directory.string()
    );

    Transaction setup_transaction(
        database,
        log_manager
    );

    const RecordId rid =
        setup_transaction.insert(
            "student",
            {
                {"id", "1"},
                {"name", "Alice"}
            }
        );

    setup_transaction.commit();

    Transaction transaction(
        database,
        log_manager
    );

    auto plan =
        create_plan(
            "UPDATE student SET name = 'Bob' WHERE id = 1;"
        );

    UpdateExecutor executor(
        *plan,
        database,
        transaction
    );

    EXPECT_EQ(
        executor.execute(),
        1u
    );

    EXPECT_EQ(
        database.open_table("student")->get(
            rid,
            "name"
        ),
        "Bob"
    );

    transaction.rollback();

    EXPECT_EQ(
        database.open_table("student")->get(
            rid,
            "name"
        ),
        "Alice"
    );
}

// Verify that DELETE executed through a transaction can be rolled back.
TEST_F(
    TransactionExecutorTest,
    RollbackDeleteExecutor) {

    Database database(
        database_directory.string()
    );

    create_student_table(database);

    LogManager log_manager(
        database_directory.string()
    );

    Transaction setup_transaction(
        database,
        log_manager
    );

    const RecordId rid =
        setup_transaction.insert(
            "student",
            {
                {"id", "1"},
                {"name", "Alice"}
            }
        );

    setup_transaction.commit();

    Transaction transaction(
        database,
        log_manager
    );

    auto plan =
        create_plan(
            "DELETE FROM student WHERE id = 1;"
        );

    DeleteExecutor executor(
        *plan,
        database,
        transaction
    );

    EXPECT_EQ(
        executor.execute(),
        1u
    );

    EXPECT_TRUE(
        database.open_table("student")->scan().empty()
    );

    transaction.rollback();

    EXPECT_EQ(
        database.open_table("student")->get(
            rid,
            "name"
        ),
        "Alice"
    );
}

} // namespace flashdb