#include <gtest/gtest.h>

#include "parser/lexer.h"
#include "parser/parser.h"
#include "planner/planner.h"

namespace flashdb {

TEST(PlannerTest, CreatesTableScanPlan) {
    Lexer lexer("SELECT name FROM student;");

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;
    const auto plan = planner.create_plan(statement);

    ASSERT_NE(plan, nullptr);

    EXPECT_EQ(plan->get_name(), "Project");
    EXPECT_EQ(plan->get_table_name(), "student");

    ASSERT_NE(plan->get_child(), nullptr);

    EXPECT_EQ(plan->get_child()->get_name(), "TableScan");
    EXPECT_EQ(plan->get_child()->get_table_name(), "student");

    ASSERT_EQ(plan->get_columns().size(), 1);

    EXPECT_EQ(
        plan->get_columns()[0].type(),
        ExpressionType::IDENTIFIER
    );

    EXPECT_EQ(
        plan->get_columns()[0].value(),
        "name"
    );
}

TEST(PlannerTest, CreatesFilterPlan) {
    Lexer lexer(
        "SELECT name FROM student WHERE id = 1;"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;
    const auto plan = planner.create_plan(statement);

    ASSERT_NE(plan, nullptr);

    EXPECT_EQ(plan->get_name(), "Project");
    EXPECT_EQ(plan->get_table_name(), "student");

    ASSERT_NE(plan->get_child(), nullptr);

    const Plan* filter = plan->get_child();

    EXPECT_EQ(filter->get_name(), "Filter");
    EXPECT_EQ(filter->get_table_name(), "student");

    ASSERT_NE(filter->get_child(), nullptr);

    EXPECT_EQ(
        filter->get_child()->get_name(),
        "TableScan"
    );

    EXPECT_EQ(
        filter->get_child()->get_table_name(),
        "student"
    );

    ASSERT_NE(filter->get_condition(), nullptr);

    EXPECT_EQ(
        filter->get_condition()->operator_(),
        "="
    );
}

TEST(PlannerTest, CreatesInsertPlan) {
    Lexer lexer(
        "INSERT INTO student VALUES (1, 'Alice');"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;
    const auto plan = planner.create_plan(statement);

    ASSERT_NE(plan, nullptr);

    EXPECT_EQ(plan->get_name(), "Insert");
    EXPECT_EQ(plan->get_table_name(), "student");

    ASSERT_EQ(plan->get_values().size(), 2);

    EXPECT_EQ(
        plan->get_values()[0].type(),
        ExpressionType::INTEGER
    );

    EXPECT_EQ(
        plan->get_values()[0].value(),
        "1"
    );

    EXPECT_EQ(
        plan->get_values()[1].type(),
        ExpressionType::STRING
    );

    EXPECT_EQ(
        plan->get_values()[1].value(),
        "Alice"
    );
}

TEST(PlannerTest, CreatesUpdatePlan) {
    Lexer lexer(
        "UPDATE student SET name = 'Charlie' WHERE id = 1;"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;
    const auto plan = planner.create_plan(statement);

    ASSERT_NE(plan, nullptr);

    EXPECT_EQ(plan->get_name(), "Update");
    EXPECT_EQ(plan->get_table_name(), "student");

    ASSERT_EQ(plan->get_columns().size(), 1);

    EXPECT_EQ(
        plan->get_columns()[0].type(),
        ExpressionType::IDENTIFIER
    );

    EXPECT_EQ(
        plan->get_columns()[0].value(),
        "name"
    );

    ASSERT_EQ(plan->get_values().size(), 1);

    EXPECT_EQ(
        plan->get_values()[0].type(),
        ExpressionType::STRING
    );

    EXPECT_EQ(
        plan->get_values()[0].value(),
        "Charlie"
    );

    ASSERT_NE(plan->get_condition(), nullptr);

    EXPECT_EQ(
        plan->get_condition()->left().value(),
        "id"
    );

    EXPECT_EQ(
        plan->get_condition()->operator_(),
        "="
    );

    EXPECT_EQ(
        plan->get_condition()->right().value(),
        "1"
    );
}

TEST(PlannerTest, CreatesUpdatePlanWithoutCondition) {
    Lexer lexer(
        "UPDATE student SET name = 'Charlie';"
    );

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    Planner planner;
    const auto plan = planner.create_plan(statement);

    ASSERT_NE(plan, nullptr);

    EXPECT_EQ(plan->get_name(), "Update");
    EXPECT_EQ(plan->get_table_name(), "student");

    ASSERT_EQ(plan->get_columns().size(), 1);

    EXPECT_EQ(
        plan->get_columns()[0].value(),
        "name"
    );

    ASSERT_EQ(plan->get_values().size(), 1);

    EXPECT_EQ(
        plan->get_values()[0].value(),
        "Charlie"
    );

    EXPECT_EQ(
        plan->get_condition(),
        nullptr
    );
}

} // namespace flashdb