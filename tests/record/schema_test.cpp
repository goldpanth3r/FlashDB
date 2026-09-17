#include <gtest/gtest.h>

#include "record/schema.h"

namespace flashdb {

TEST(SchemaTest, AddsIntegerField) {
    Schema schema;

    schema.add_int_field("id");

    ASSERT_EQ(schema.field_count(), 1);
    EXPECT_EQ(schema.fields()[0].name, "id");
    EXPECT_EQ(schema.fields()[0].type, FieldType::INT);
    EXPECT_EQ(schema.fields()[0].length, sizeof(int32_t));
}

TEST(SchemaTest, AddsStringField) {
    Schema schema;

    schema.add_string_field("name", 50);

    ASSERT_EQ(schema.field_count(), 1);
    EXPECT_EQ(schema.fields()[0].name, "name");
    EXPECT_EQ(schema.fields()[0].type, FieldType::STRING);
    EXPECT_EQ(schema.fields()[0].length, 50);
}

TEST(SchemaTest, RejectsEmptyFieldName) {
    Schema schema;

    EXPECT_THROW(
        schema.add_int_field(""),
        std::invalid_argument
    );
}

TEST(SchemaTest, RejectsZeroStringLength) {
    Schema schema;

    EXPECT_THROW(
        schema.add_string_field("name", 0),
        std::invalid_argument
    );
}

} // namespace flashdb