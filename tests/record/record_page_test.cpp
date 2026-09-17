#include <gtest/gtest.h>

#include "record/record_page.h"

namespace flashdb {

TEST(RecordPageTest, InsertsAndReadsRecord) {
    Schema schema;

    schema.add_int_field("id");
    schema.add_string_field("name", 20);

    Layout layout(schema);

    Page page;
    RecordPage records(page, layout);

    const std::size_t slot =
        records.insert({
            {"id", "42"},
            {"name", "Alice"}
        });

    EXPECT_TRUE(records.is_used(slot));
    EXPECT_EQ(records.get(slot, "id"), "42");
    EXPECT_EQ(records.get(slot, "name"), "Alice");
}

TEST(RecordPageTest, UpdatesRecord) {
    Schema schema;

    schema.add_int_field("id");
    schema.add_string_field("name", 20);

    Layout layout(schema);

    Page page;
    RecordPage records(page, layout);

    const std::size_t slot =
        records.insert({
            {"id", "42"},
            {"name", "Alice"}
        });

    records.set(slot, "name", "Bob");

    EXPECT_EQ(records.get(slot, "name"), "Bob");
}

TEST(RecordPageTest, DeletesRecord) {
    Schema schema;

    schema.add_int_field("id");
    schema.add_string_field("name", 20);

    Layout layout(schema);

    Page page;
    RecordPage records(page, layout);

    const std::size_t slot =
        records.insert({
            {"id", "42"},
            {"name", "Alice"}
        });

    records.remove(slot);

    EXPECT_FALSE(records.is_used(slot));
}

TEST(RecordPageTest, ReusesDeletedSlot) {
    Schema schema;

    schema.add_int_field("id");

    Layout layout(schema);

    Page page;
    RecordPage records(page, layout);

    const std::size_t first =
        records.insert({
            {"id", "1"}
        });

    records.remove(first);

    const std::size_t second =
        records.insert({
            {"id", "2"}
        });

    EXPECT_EQ(first, second);
    EXPECT_EQ(records.get(second, "id"), "2");
}

TEST(RecordPageTest, RejectsOversizedString) {
    Schema schema;

    schema.add_string_field("name", 5);

    Layout layout(schema);

    Page page;
    RecordPage records(page, layout);

    EXPECT_THROW(
        records.insert({
            {"name", "abcdef"}
        }),
        std::length_error
    );
}

TEST(RecordPageTest, RejectsMissingField) {
    Schema schema;

    schema.add_int_field("id");
    schema.add_string_field("name", 20);

    Layout layout(schema);

    Page page;
    RecordPage records(page, layout);

    EXPECT_THROW(
        records.insert({
            {"id", "42"}
        }),
        std::invalid_argument
    );
}

TEST(RecordPageTest, DetectsFreeSlot) {
    Schema schema;
    schema.add_int_field("id");
    schema.add_string_field("name", 20);

    Layout layout(schema);
    Page page;
    RecordPage record_page(page, layout);

    EXPECT_TRUE(record_page.has_free_slot());

    record_page.insert({
        {"id", "1"},
        {"name", "Alice"}
    });

    EXPECT_TRUE(record_page.has_free_slot());
}

} // namespace flashdb