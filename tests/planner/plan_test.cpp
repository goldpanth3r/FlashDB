#include <gtest/gtest.h>
#include <memory>

#include "planner/plan.h"

namespace flashdb {

TEST(PlanTest, StoresPlanName) {
    Plan plan("TableScan", "student");

    EXPECT_EQ(plan.get_name(), "TableScan");
}

TEST(PlanTest, StoresTableName) {
    Plan plan("TableScan", "student");

    EXPECT_EQ(plan.get_table_name(), "student");
}

TEST(PlanTest, StoresChildPlan) {
    auto child = std::make_unique<Plan>("TableScan", "student");

    Plan plan(
        "Filter",
        "student",
        std::move(child)
    );

    ASSERT_NE(plan.get_child(), nullptr);

    EXPECT_EQ(plan.get_child()->get_name(), "TableScan");
    EXPECT_EQ(plan.get_child()->get_table_name(), "student");
}

} // namespace flashdb