#include <gtest/gtest.h>

#include "record/layout.h"

namespace flashdb {

TEST(LayoutTest, CalculatesFieldOffsets) {
    Schema schema;

    schema.add_int_field("id");
    schema.add_string_field("name", 50);
    schema.add_int_field("age");

    Layout layout(schema);

    EXPECT_EQ(layout.offset("id"), 0);
    EXPECT_EQ(layout.offset("name"), sizeof(int32_t));
    EXPECT_EQ(
        layout.offset("age"),
        sizeof(int32_t) + 50
    );
}

TEST(LayoutTest, CalculatesFieldSizes) {
    Schema schema;

    schema.add_int_field("id");
    schema.add_string_field("name", 50);

    Layout layout(schema);

    EXPECT_EQ(
        layout.field_size("id"),
        sizeof(int32_t)
    );

    EXPECT_EQ(
        layout.field_size("name"),
        50
    );
}

TEST(LayoutTest, CalculatesRecordSize) {
    Schema schema;

    schema.add_int_field("id");
    schema.add_string_field("name", 50);
    schema.add_int_field("age");

    Layout layout(schema);

    EXPECT_EQ(
        layout.record_size(),
        sizeof(int32_t) + 50 + sizeof(int32_t)
    );
}

TEST(LayoutTest, RejectsUnknownField) {
    Schema schema;

    schema.add_int_field("id");

    Layout layout(schema);

    EXPECT_THROW(
        layout.offset("unknown"),
        std::out_of_range
    );

    EXPECT_THROW(
        layout.field_size("unknown"),
        std::out_of_range
    );
}

TEST(LayoutTest, RejectsDuplicateFieldNames) {
    Schema schema;

    schema.add_int_field("id");
    schema.add_string_field("id", 20);

    EXPECT_THROW(
        Layout layout(schema),
        std::invalid_argument
    );
}

} // namespace flashdb