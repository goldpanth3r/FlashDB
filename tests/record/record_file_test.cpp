#include <filesystem>
#include <string>
#include <unordered_map>

#include <gtest/gtest.h>

#include "buffer/buffer_manager.h"
#include "file/file_manager.h"
#include "record/layout.h"
#include "record/record_file.h"
#include "record/schema.h"

namespace flashdb {

class RecordFileTest : public ::testing::Test {
protected:
    std::filesystem::path test_directory;

    void SetUp() override {
        test_directory =
            std::filesystem::temp_directory_path()
            / "flashdb_record_file_test";

        std::filesystem::remove_all(test_directory);
        std::filesystem::create_directories(test_directory);
    }

    void TearDown() override {
        std::filesystem::remove_all(test_directory);
    }
};

/**
 * createRecordFile creates the database components needed by a test.
 *
 * @param file_manager File manager for disk I/O.
 * @param buffer_manager Buffer manager for cached pages.
 * @return A configured RecordFile.
 */
static RecordFile createRecordFile(
    FileManager& file_manager,
    BufferManager& buffer_manager) {

    Schema schema;

    schema.add_int_field("id");
    schema.add_string_field("name", 50);

    Layout layout(schema);

    return RecordFile(
        file_manager,
        buffer_manager,
        "student",
        layout
    );
}

TEST_F(RecordFileTest, InsertsAndReadsRecord) {

    FileManager file_manager(
        test_directory.string()
    );

    BufferManager buffer_manager(
        file_manager,
        3
    );

    Schema schema;
    schema.add_int_field("id");
    schema.add_string_field("name", 50);

    Layout layout(schema);

    RecordFile records(
        file_manager,
        buffer_manager,
        "student",
        layout
    );

    const RecordId rid =
        records.insert({
            {"id", "1"},
            {"name", "Alice"}
        });

    EXPECT_EQ(
        records.get(rid, "id"),
        "1"
    );

    EXPECT_EQ(
        records.get(rid, "name"),
        "Alice"
    );
}

TEST_F(RecordFileTest, UpdatesRecord) {

    FileManager file_manager(
        test_directory.string()
    );

    BufferManager buffer_manager(
        file_manager,
        3
    );

    Schema schema;
    schema.add_int_field("id");
    schema.add_string_field("name", 50);

    Layout layout(schema);

    RecordFile records(
        file_manager,
        buffer_manager,
        "student",
        layout
    );

    const RecordId rid =
        records.insert({
            {"id", "1"},
            {"name", "Alice"}
        });

    records.set(
        rid,
        "name",
        "Bob"
    );

    EXPECT_EQ(
        records.get(rid, "name"),
        "Bob"
    );
}

TEST_F(RecordFileTest, RemovesRecord) {

    FileManager file_manager(
        test_directory.string()
    );

    BufferManager buffer_manager(
        file_manager,
        3
    );

    Schema schema;
    schema.add_int_field("id");
    schema.add_string_field("name", 50);

    Layout layout(schema);

    RecordFile records(
        file_manager,
        buffer_manager,
        "student",
        layout
    );

    const RecordId rid =
        records.insert({
            {"id", "1"},
            {"name", "Alice"}
        });

    records.remove(rid);

    EXPECT_THROW(
        records.get(rid, "name"),
        std::runtime_error
    );
}

TEST_F(RecordFileTest, InsertsRecordsIntoMultiplePages) {

    FileManager file_manager(
        test_directory.string()
    );

    BufferManager buffer_manager(
        file_manager,
        5
    );

    Schema schema;
    schema.add_int_field("id");
    schema.add_string_field("name", 50);

    Layout layout(schema);

    RecordFile records(
        file_manager,
        buffer_manager,
        "student",
        layout
    );

    const std::size_t slots_per_page =
        Page::PAGE_SIZE /
        (1 + layout.record_size());

    const std::size_t record_count =
        slots_per_page + 1;

    RecordId last_rid(0, 0);

    for (std::size_t i = 0;
         i < record_count;
         ++i) {

        last_rid =
            records.insert({
                {"id", std::to_string(i)},
                {"name", "Student"}
            });
    }

    // The final record must be on the second page.
    EXPECT_EQ(
        last_rid.page_number(),
        1
    );

    EXPECT_EQ(
        records.get(last_rid, "id"),
        std::to_string(record_count - 1)
    );
}


TEST_F(RecordFileTest, PersistsRecordAfterRestart) {
    RecordId rid(0, 0);

    // First database session.
    {
        FileManager file_manager(test_directory.string());
        BufferManager buffer_manager(file_manager, 3);

        Schema schema;
        schema.add_int_field("id");
        schema.add_string_field("name", 50);
        Layout layout(schema);

        RecordFile records(
            file_manager,
            buffer_manager,
            "student",
            layout
        );

        rid = records.insert({
            {"id", "1"},
            {"name", "Alice"}
        });

        // Write dirty buffer data to disk.
        buffer_manager.flush_all();
    }

    // Second database session.
    {
        FileManager file_manager(test_directory.string());
        BufferManager buffer_manager(file_manager, 3);

        Schema schema;
        schema.add_int_field("id");
        schema.add_string_field("name", 50);
        Layout layout(schema);

        RecordFile records(
            file_manager,
            buffer_manager,
            "student",
            layout
        );

        EXPECT_EQ(records.get(rid, "id"), "1");
        EXPECT_EQ(records.get(rid, "name"), "Alice");
    }
}

TEST_F(RecordFileTest, ReusesDeletedSlot) {
    FileManager file_manager(test_directory.string());
    BufferManager buffer_manager(file_manager, 3);

    Schema schema;
    schema.add_int_field("id");
    schema.add_string_field("name", 50);
    Layout layout(schema);

    RecordFile records(
        file_manager,
        buffer_manager,
        "student",
        layout
    );

    const RecordId first =
        records.insert({
            {"id", "1"},
            {"name", "Alice"}
        });

    records.remove(first);

    const RecordId second =
        records.insert({
            {"id", "2"},
            {"name", "Bob"}
        });

    EXPECT_EQ(second.page_number(), first.page_number());
    EXPECT_EQ(second.slot_number(), first.slot_number());

    EXPECT_EQ(records.get(second, "id"), "2");
    EXPECT_EQ(records.get(second, "name"), "Bob");
}

TEST_F(RecordFileTest, InvalidInsertDoesNotConsumeSlot) {
    FileManager file_manager(test_directory.string());
    BufferManager buffer_manager(file_manager, 3);

    Schema schema;
    schema.add_int_field("id");
    schema.add_string_field("name", 20);
    Layout layout(schema);

    RecordFile records(
        file_manager,
        buffer_manager,
        "student",
        layout
    );

    EXPECT_THROW(
        records.insert({
            {"id", "not-a-number"},
            {"name", "Alice"}
        }),
        std::invalid_argument
    );

    const RecordId rid =
        records.insert({
            {"id", "1"},
            {"name", "Bob"}
        });

    EXPECT_EQ(rid.page_number(), 0);
    EXPECT_EQ(rid.slot_number(), 0);
    EXPECT_EQ(records.get(rid, "name"), "Bob");
}

TEST_F(RecordFileTest, ReportsPageCount) {
    FileManager file_manager(test_directory.string());
    BufferManager buffer_manager(file_manager, 3);

    Schema schema;
    schema.add_int_field("id");
    schema.add_string_field("name", 50);
    Layout layout(schema);

    RecordFile records(
        file_manager,
        buffer_manager,
        "student",
        layout
    );

    EXPECT_EQ(records.page_count(), 0);

    records.insert({
        {"id", "1"},
        {"name", "Alice"}
    });

    EXPECT_EQ(records.page_count(), 1);
}

TEST_F(RecordFileTest, ScansAllRecords) {
    FileManager file_manager(test_directory.string());
    BufferManager buffer_manager(file_manager, 3);

    Schema schema;
    schema.add_int_field("id");
    schema.add_string_field("name", 50);
    Layout layout(schema);

    RecordFile records(
        file_manager,
        buffer_manager,
        "student",
        layout
    );

    const RecordId first =
        records.insert({
            {"id", "1"},
            {"name", "Alice"}
        });

    const RecordId second =
        records.insert({
            {"id", "2"},
            {"name", "Bob"}
        });

    records.insert({
        {"id", "3"},
        {"name", "Charlie"}
    });

    const auto result = records.scan();

    ASSERT_EQ(result.size(), 3);

    EXPECT_EQ(result[0], first);
    EXPECT_EQ(result[1], second);

    EXPECT_EQ(records.get(result[2], "name"), "Charlie");
}

TEST_F(RecordFileTest, InvalidInsertDoesNotCreateNewPage) {
    FileManager file_manager(test_directory.string());
    BufferManager buffer_manager(file_manager, 5);

    Schema schema;
    schema.add_int_field("id");
    schema.add_string_field("name", 50);
    Layout layout(schema);

    RecordFile records(
        file_manager,
        buffer_manager,
        "student",
        layout
    );

    const std::size_t slots_per_page =
        Page::PAGE_SIZE /
        (1 + layout.record_size());

    for (std::size_t i = 0;
         i < slots_per_page;
         ++i) {

        records.insert({
            {"id", std::to_string(i)},
            {"name", "Student"}
        });
    }

    EXPECT_EQ(records.page_count(), 1);

    EXPECT_THROW(
        records.insert({
            {"id", "not-a-number"},
            {"name", "Invalid"}
        }),
        std::invalid_argument
    );

    EXPECT_EQ(records.page_count(), 1);
}

} // namespace flashdb