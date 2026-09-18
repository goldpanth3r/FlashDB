#include <filesystem>
#include <stdexcept>

#include <gtest/gtest.h>

#include "metadata/table_catalog.h"

namespace flashdb {

class TableCatalogTest : public ::testing::Test {
protected:
    void SetUp() override {
        catalog_file =
            std::filesystem::temp_directory_path()
            / "flashdb_table_catalog_test.catalog";

        std::filesystem::remove(catalog_file);
    }

    void TearDown() override {
        std::filesystem::remove(catalog_file);
    }

    std::filesystem::path catalog_file;
};

TEST_F(TableCatalogTest, CreatesAndFindsTable) {
    TableCatalog catalog(
        catalog_file.string()
    );

    Schema schema;

    schema.add_int_field("id");
    schema.add_string_field("name", 50);

    catalog.create_table(
        "student",
        schema
    );

    EXPECT_TRUE(
        catalog.has_table("student")
    );

    EXPECT_EQ(
        catalog.table_count(),
        1u
    );

    const Schema& stored =
        catalog.get_schema("student");

    ASSERT_EQ(
        stored.field_count(),
        2u
    );

    EXPECT_EQ(
        stored.fields()[0].name,
        "id"
    );

    EXPECT_EQ(
        stored.fields()[0].type,
        FieldType::INT
    );

    EXPECT_EQ(
        stored.fields()[1].name,
        "name"
    );

    EXPECT_EQ(
        stored.fields()[1].type,
        FieldType::STRING
    );

    EXPECT_EQ(
        stored.fields()[1].length,
        50u
    );
}

TEST_F(TableCatalogTest, SavesAndLoadsTableMetadata) {
    {
        TableCatalog catalog(
            catalog_file.string()
        );

        Schema schema;

        schema.add_int_field("id");
        schema.add_string_field("name", 100);

        catalog.create_table(
            "student",
            schema
        );

        catalog.save();
    }

    {
        TableCatalog catalog(
            catalog_file.string()
        );

        catalog.load();

        ASSERT_TRUE(
            catalog.has_table("student")
        );

        EXPECT_EQ(
            catalog.table_count(),
            1u
        );

        const Schema& schema =
            catalog.get_schema("student");

        ASSERT_EQ(
            schema.field_count(),
            2u
        );

        EXPECT_EQ(
            schema.fields()[0].name,
            "id"
        );

        EXPECT_EQ(
            schema.fields()[0].type,
            FieldType::INT
        );

        EXPECT_EQ(
            schema.fields()[1].name,
            "name"
        );

        EXPECT_EQ(
            schema.fields()[1].type,
            FieldType::STRING
        );

        EXPECT_EQ(
            schema.fields()[1].length,
            100u
        );
    }
}

TEST_F(TableCatalogTest, LoadsMultipleTables) {
    {
        TableCatalog catalog(
            catalog_file.string()
        );

        Schema student;

        student.add_int_field("id");
        student.add_string_field("name", 50);

        Schema course;

        course.add_int_field("id");
        course.add_string_field("title", 100);

        catalog.create_table(
            "student",
            student
        );

        catalog.create_table(
            "course",
            course
        );

        catalog.save();
    }

    TableCatalog catalog(
        catalog_file.string()
    );

    catalog.load();

    EXPECT_EQ(
        catalog.table_count(),
        2u
    );

    EXPECT_TRUE(
        catalog.has_table("student")
    );

    EXPECT_TRUE(
        catalog.has_table("course")
    );

    EXPECT_EQ(
        catalog.get_schema("student")
            .fields()[1].length,
        50u
    );

    EXPECT_EQ(
        catalog.get_schema("course")
            .fields()[1].length,
        100u
    );
}

TEST_F(TableCatalogTest, MissingCatalogStartsEmpty) {
    TableCatalog catalog(
        catalog_file.string()
    );

    catalog.load();

    EXPECT_EQ(
        catalog.table_count(),
        0u
    );
}

TEST_F(TableCatalogTest, RejectsDuplicateTable) {
    TableCatalog catalog(
        catalog_file.string()
    );

    Schema schema;

    schema.add_int_field("id");

    catalog.create_table(
        "student",
        schema
    );

    EXPECT_THROW(
        catalog.create_table(
            "student",
            schema
        ),
        std::invalid_argument
    );
}

}