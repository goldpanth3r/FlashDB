#include <gtest/gtest.h>

#include "index/index_manager.h"

namespace flashdb {

TEST(IndexManagerTest, StartsWithoutIndexes) {
    IndexManager manager;

    EXPECT_FALSE(manager.has_index("student_id"));
}

TEST(IndexManagerTest, CreatesIndex) {
    IndexManager manager;

    manager.create_index("student_id");

    EXPECT_TRUE(manager.has_index("student_id"));
}

TEST(IndexManagerTest, CreatingSameIndexTwiceIsSafe) {
    IndexManager manager;

    manager.create_index("student_id");
    manager.create_index("student_id");

    EXPECT_TRUE(manager.has_index("student_id"));
}

TEST(IndexManagerTest, ReturnsExistingIndex) {
    IndexManager manager;

    manager.create_index("student_id");

    BPlusTree& tree = manager.index("student_id");

    tree.insert(10, RecordId(0, 3));

    RecordId rid(0, 0);

    EXPECT_TRUE(tree.search(10, rid));
    EXPECT_EQ(rid.page_number(), 0);
    EXPECT_EQ(rid.slot_number(), 3u);
}

TEST(IndexManagerTest, MissingIndexThrows) {
    IndexManager manager;

    EXPECT_THROW(
        manager.index("missing"),
        std::runtime_error
    );
}

TEST(IndexManagerTest, RemovesSpecificDuplicateRecord) {
    IndexManager manager;

    manager.create_index(
        "student_id",
        "student",
        "id"
    );

    const RecordId first(0, 1);
    const RecordId second(0, 2);

    manager.insert(
        "student_id",
        10,
        first
    );

    manager.insert(
        "student_id",
        10,
        second
    );

    EXPECT_TRUE(
        manager.remove(
            "student_id",
            10,
            second
        )
    );

    const std::vector<RecordId> results =
        manager.search_all(
            "student_id",
            10
        );

    EXPECT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0], first);
}

} // namespace flashdb