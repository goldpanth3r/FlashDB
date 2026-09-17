#include <gtest/gtest.h>

#include "file/page.h"

using namespace flashdb;

TEST(PageTest, StoresInteger) {
    Page page;

    page.set_int(0, 42);

    EXPECT_EQ(page.get_int(0), 42);
}

TEST(PageTest, StoresString) {
    Page page;

    page.set_string(100, "Alice");

    EXPECT_EQ(page.get_string(100), "Alice");
}

TEST(PageTest, PageSizeIs4096) {
    Page page;

    EXPECT_EQ(page.data().size(), Page::PAGE_SIZE);
}