#include <filesystem>
#include <stdexcept>

#include <gtest/gtest.h>

#include "database.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "planner/planner.h"
#include "query/insert_executor.h"

namespace flashdb {

class InsertExecutorTest : public ::testing::Test {
protected:
    std::filesystem::path database_directory;

    void SetUp() override {
        database_directory =
            std::filesystem::temp_directory_path()
            / "flashdb_insert_executor_test";

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

        database.file_manager().append(
            "student.tbl"
        );
    }
};

TEST_F(InsertExecutorTest, InsertsRecord) {
    Database database(
        database_directory.string()
    );

    create_student_table(database);

    Lexer lexer(
        "INSERT INTO student VALUES (1, 'Alice');"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;

    const auto plan =
        planner.create_plan(statement);

    ASSERT_NE(plan, nullptr);

    // Execute INSERT through the shared database.
    InsertExecutor executor(
        *plan,
        database
    );

    const RecordId rid =
        executor.execute();

    auto records =
        database.open_table("student");

    EXPECT_EQ(
        records->get(rid, "id"),
        "1"
    );

    EXPECT_EQ(
        records->get(rid, "name"),
        "Alice"
    );
}

TEST_F(InsertExecutorTest, InsertsMultipleRecords) {
    Database database(
        database_directory.string()
    );

    create_student_table(database);

    Planner planner;

    Lexer lexer1(
        "INSERT INTO student VALUES (1, 'Alice');"
    );

    const auto tokens1 = lexer1.tokenize();
    Parser parser1(tokens1);

    const auto plan1 =
        planner.create_plan(parser1.parse());

    ASSERT_NE(plan1, nullptr);

    InsertExecutor executor1(
        *plan1,
        database
    );

    executor1.execute();

    Lexer lexer2(
        "INSERT INTO student VALUES (2, 'Bob');"
    );

    const auto tokens2 = lexer2.tokenize();
    Parser parser2(tokens2);

    const auto plan2 =
        planner.create_plan(parser2.parse());

    ASSERT_NE(plan2, nullptr);

    InsertExecutor executor2(
        *plan2,
        database
    );

    executor2.execute();

    auto records =
        database.open_table("student");

    const auto record_ids =
        records->scan();

    ASSERT_EQ(record_ids.size(), 2);

    EXPECT_EQ(
        records->get(record_ids[0], "name"),
        "Alice"
    );

    EXPECT_EQ(
        records->get(record_ids[1], "name"),
        "Bob"
    );
}

TEST_F(InsertExecutorTest, RejectsWrongValueCount) {
    Database database(
        database_directory.string()
    );

    create_student_table(database);

    Lexer lexer(
        "INSERT INTO student VALUES (1);"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto plan =
        Planner().create_plan(parser.parse());

    ASSERT_NE(plan, nullptr);

    // Reject INSERT statements with the wrong number of values.
    InsertExecutor executor(
        *plan,
        database
    );

    EXPECT_THROW(
        executor.execute(),
        std::invalid_argument
    );
}

TEST_F(InsertExecutorTest, RejectsWrongValueType) {
    Database database(
        database_directory.string()
    );

    create_student_table(database);

    Lexer lexer(
        "INSERT INTO student VALUES ('wrong', 'Alice');"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto plan =
        Planner().create_plan(parser.parse());

    ASSERT_NE(plan, nullptr);

    // Validate column types before writing to storage.
    InsertExecutor executor(
        *plan,
        database
    );

    EXPECT_THROW(
        executor.execute(),
        std::invalid_argument
    );
}

} // namespace flashdb