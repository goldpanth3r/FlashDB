#include <gtest/gtest.h>

#include "metadata/table_catalog.h"

namespace flashdb {

TEST(TableCatalogTest, CreatesTable) {
    TableCatalog catalog;

    Schema schema;
    schema.add_int_field("id");
    schema.add_string_field("name", 50);

    catalog.create_table("student", schema);

    EXPECT_TRUE(catalog.has_table("student"));
    EXPECT_EQ(catalog.table_count(), 1);
}

TEST(TableCatalogTest, ReturnsSchema) {
    TableCatalog catalog;

    Schema schema;
    schema.add_int_field("id");
    schema.add_string_field("name", 50);

    catalog.create_table("student", schema);

    const Schema& result =
        catalog.get_schema("student");

    EXPECT_EQ(result.field_count(), 2);
    EXPECT_EQ(result.fields()[0].name, "id");
    EXPECT_EQ(result.fields()[1].name, "name");
}

TEST(TableCatalogTest, RejectsDuplicateTable) {
    TableCatalog catalog;

    Schema schema;
    schema.add_int_field("id");

    catalog.create_table("student", schema);

    EXPECT_THROW(
        catalog.create_table("student", schema),
        std::invalid_argument
    );
}

TEST(TableCatalogTest, RejectsUnknownTable) {
    TableCatalog catalog;

    EXPECT_THROW(
        catalog.get_schema("missing"),
        std::out_of_range
    );
}

} // namespace flashdb