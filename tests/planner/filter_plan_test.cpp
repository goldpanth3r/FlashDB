#include <gtest/gtest.h>

#include "parser/lexer.h"
#include "parser/parser.h"
#include "planner/filter_plan.h"

namespace flashdb {

TEST(FilterPlanTest, StoresCondition) {
    Lexer lexer("SELECT name FROM student WHERE id = 1;");

    const auto tokens = lexer.tokenize();
    Parser parser(tokens);

    const auto statement = parser.parse();

    const auto& select = std::get<SelectStatement>(statement);

    ASSERT_TRUE(select.has_condition());

    FilterPlan plan(select.condition());

    EXPECT_EQ(plan.get_condition().operator_(), "=");
}

} // namespace flashdb