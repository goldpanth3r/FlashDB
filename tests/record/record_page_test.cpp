#include <gtest/gtest.h>

#include "file/page.h"
#include "record/record_page.h"
#include "record/schema.h"

using namespace flashdb;

TEST(RecordPageTest, InsertsAndReadsRecord) {
    Page page;

    Schema schema;
    schema.add_int_field("id");
    schema.add_string_field("name", 20);

    RecordPage records(page, schema);

    const int slot =
        records.insert("1Alice");

    EXPECT_EQ(slot, 0);
    EXPECT_EQ(records.get(0).substr(0, 6), "1Alice");
}

TEST(RecordPageTest, MultipleRecordsGetDifferentSlots) {
    Page page;

    Schema schema;
    schema.add_string_field("name", 20);

    RecordPage records(page, schema);

    EXPECT_EQ(records.insert("Alice"), 0);
    EXPECT_EQ(records.insert("Bob"), 1);

    EXPECT_EQ(records.get(0).substr(0, 5), "Alice");
    EXPECT_EQ(records.get(1).substr(0, 3), "Bob");
}