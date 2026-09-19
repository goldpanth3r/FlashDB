#include <gtest/gtest.h>

#include "index/b_plus_tree.h"

namespace flashdb {

TEST(BPlusTreeTest, StartsEmpty) {
    BPlusTree tree;

    EXPECT_EQ(tree.size(), 0);
}

TEST(BPlusTreeTest, InsertsAndFindsKey) {
    BPlusTree tree;

    RecordId rid(3, 7);

    tree.insert(10, rid);

    RecordId result(0, 0);

    EXPECT_TRUE(tree.search(10, result));
    EXPECT_EQ(result, rid);
}

TEST(BPlusTreeTest, ReturnsFalseForMissingKey) {
    BPlusTree tree;

    RecordId rid(1, 2);

    tree.insert(10, rid);

    RecordId result(0, 0);

    EXPECT_FALSE(tree.search(20, result));
}

TEST(BPlusTreeTest, RemovesKey) {
    BPlusTree tree;

    RecordId rid(2, 4);

    tree.insert(10, rid);

    EXPECT_TRUE(tree.remove(10));
    EXPECT_EQ(tree.size(), 0);

    RecordId result(0, 0);

    EXPECT_FALSE(tree.search(10, result));
}

TEST(BPlusTreeTest, RangeScanReturnsSortedRecords) {
    BPlusTree tree;

    tree.insert(30, RecordId(3, 0));
    tree.insert(10, RecordId(1, 0));
    tree.insert(20, RecordId(2, 0));
    tree.insert(40, RecordId(4, 0));

    const auto records =
        tree.range_scan(15, 35);

    ASSERT_EQ(records.size(), 2);

    EXPECT_EQ(records[0], RecordId(2, 0));
    EXPECT_EQ(records[1], RecordId(3, 0));
}

TEST(BPlusTreeTest, EmptyRangeReturnsNothing) {
    BPlusTree tree;

    tree.insert(10, RecordId(1, 0));
    tree.insert(20, RecordId(2, 0));

    const auto records =
        tree.range_scan(30, 40);

    EXPECT_TRUE(records.empty());
}

TEST(BPlusTreeTest, SplitsRootLeaf) {
    BPlusTree tree;

    tree.insert(10, RecordId(1, 0));
    tree.insert(20, RecordId(2, 0));
    tree.insert(30, RecordId(3, 0));
    tree.insert(40, RecordId(4, 0));

    EXPECT_EQ(tree.size(), 4);

    RecordId rid(0, 0);

    EXPECT_TRUE(tree.search(10, rid));
    EXPECT_EQ(rid, RecordId(1, 0));

    EXPECT_TRUE(tree.search(20, rid));
    EXPECT_EQ(rid, RecordId(2, 0));

    EXPECT_TRUE(tree.search(30, rid));
    EXPECT_EQ(rid, RecordId(3, 0));

    EXPECT_TRUE(tree.search(40, rid));
    EXPECT_EQ(rid, RecordId(4, 0));
}

TEST(BPlusTreeTest, RangeScanAcrossLeaves) {
    BPlusTree tree;

    tree.insert(10, RecordId(1, 0));
    tree.insert(20, RecordId(2, 0));
    tree.insert(30, RecordId(3, 0));
    tree.insert(40, RecordId(4, 0));

    const auto result =
        tree.range_scan(20, 40);

    ASSERT_EQ(result.size(), 3);

    EXPECT_EQ(result[0], RecordId(2, 0));
    EXPECT_EQ(result[1], RecordId(3, 0));
    EXPECT_EQ(result[2], RecordId(4, 0));
}

TEST(BPlusTreeTest, InsertsIntoCorrectLeafAfterSplit) {
    BPlusTree tree;

    tree.insert(10, RecordId(1, 0));
    tree.insert(20, RecordId(2, 0));
    tree.insert(30, RecordId(3, 0));
    tree.insert(40, RecordId(4, 0));

    tree.insert(50, RecordId(5, 0));

    EXPECT_EQ(tree.size(), 5);

    RecordId rid(0, 0);

    EXPECT_TRUE(tree.search(50, rid));
    EXPECT_EQ(rid, RecordId(5, 0));

    const auto result =
        tree.range_scan(10, 50);

    ASSERT_EQ(result.size(), 5);

    EXPECT_EQ(result[0], RecordId(1, 0));
    EXPECT_EQ(result[1], RecordId(2, 0));
    EXPECT_EQ(result[2], RecordId(3, 0));
    EXPECT_EQ(result[3], RecordId(4, 0));
    EXPECT_EQ(result[4], RecordId(5, 0));
}

TEST(BPlusTreeTest, SplitsMultipleLeaves) {
    BPlusTree tree;

    for (int key = 10; key <= 60; key += 10) {
        tree.insert(
            key,
            RecordId(key, 0)
        );
    }

    EXPECT_EQ(tree.size(), 6);

    for (int key = 10; key <= 60; key += 10) {
        RecordId rid(0, 0);

        EXPECT_TRUE(
            tree.search(key, rid)
        );

        EXPECT_EQ(
            rid,
            RecordId(key, 0)
        );
    }
}

TEST(BPlusTreeTest, RangeScanAcrossMultipleLeaves) {
    BPlusTree tree;

    for (int key = 10; key <= 60; key += 10) {
        tree.insert(
            key,
            RecordId(key, 0)
        );
    }

    const auto result =
        tree.range_scan(20, 50);

    ASSERT_EQ(result.size(), 4);

    EXPECT_EQ(result[0], RecordId(20, 0));
    EXPECT_EQ(result[1], RecordId(30, 0));
    EXPECT_EQ(result[2], RecordId(40, 0));
    EXPECT_EQ(result[3], RecordId(50, 0));
}

TEST(BPlusTreeTest, HandlesManyKeys) {
    BPlusTree tree;

    for (int key = 1; key <= 50; ++key) {
        tree.insert(
            key,
            RecordId(key, 0)
        );
    }

    EXPECT_EQ(tree.size(), 50);

    for (int key = 1; key <= 50; ++key) {
        RecordId rid(0, 0);

        EXPECT_TRUE(
            tree.search(key, rid)
        );

        EXPECT_EQ(
            rid,
            RecordId(key, 0)
        );
    }
}

TEST(BPlusTreeTest, HandlesReverseInsertOrder) {
    BPlusTree tree;

    for (int key = 50; key >= 1; --key) {
        tree.insert(
            key,
            RecordId(key, 0)
        );
    }

    EXPECT_EQ(tree.size(), 50);

    for (int key = 1; key <= 50; ++key) {
        RecordId rid(0, 0);

        EXPECT_TRUE(
            tree.search(key, rid)
        );

        EXPECT_EQ(
            rid,
            RecordId(key, 0)
        );
    }
}

TEST(BPlusTreeTest, RangeScanHandlesManyLeaves) {
    BPlusTree tree;

    for (int key = 1; key <= 50; ++key) {
        tree.insert(
            key,
            RecordId(key, 0)
        );
    }

    const auto result =
        tree.range_scan(15, 35);

    ASSERT_EQ(result.size(), 21);

    for (std::size_t i = 0; i < result.size(); ++i) {
        EXPECT_EQ(
            result[i],
            RecordId(
                static_cast<int>(15 + i),
                0
            )
        );
    }
}

TEST(BPlusTreeTest, RemovesKeyFromMultipleLeaves) {
    BPlusTree tree;

    for (int key = 1; key <= 20; ++key) {
        tree.insert(
            key,
            RecordId(key, 0)
        );
    }

    EXPECT_EQ(tree.size(), 20);

    EXPECT_TRUE(tree.remove(1));
    EXPECT_TRUE(tree.remove(10));
    EXPECT_TRUE(tree.remove(20));

    EXPECT_EQ(tree.size(), 17);

    RecordId rid(0, 0);

    EXPECT_FALSE(tree.search(1, rid));
    EXPECT_FALSE(tree.search(10, rid));
    EXPECT_FALSE(tree.search(20, rid));

    EXPECT_TRUE(tree.search(2, rid));
    EXPECT_EQ(rid, RecordId(2, 0));

    EXPECT_TRUE(tree.search(11, rid));
    EXPECT_EQ(rid, RecordId(11, 0));
}

TEST(BPlusTreeTest, RangeScanAfterDeletion) {
    BPlusTree tree;

    for (int key = 1; key <= 20; ++key) {
        tree.insert(
            key,
            RecordId(key, 0)
        );
    }

    EXPECT_TRUE(tree.remove(5));
    EXPECT_TRUE(tree.remove(6));
    EXPECT_TRUE(tree.remove(7));

    const auto result =
        tree.range_scan(1, 10);

    ASSERT_EQ(result.size(), 7);

    EXPECT_EQ(result[0], RecordId(1, 0));
    EXPECT_EQ(result[1], RecordId(2, 0));
    EXPECT_EQ(result[2], RecordId(3, 0));
    EXPECT_EQ(result[3], RecordId(4, 0));
    EXPECT_EQ(result[4], RecordId(8, 0));
    EXPECT_EQ(result[5], RecordId(9, 0));
    EXPECT_EQ(result[6], RecordId(10, 0));
}

TEST(BPlusTreeTest, DeletesUsingLeafBorrow) {
    BPlusTree tree;

    for (int key = 10; key <= 50; key += 10) {
        tree.insert(
            key,
            RecordId(key, 0)
        );
    }

    EXPECT_TRUE(tree.remove(30));

    EXPECT_EQ(tree.size(), 4);

    RecordId rid(0, 0);

    EXPECT_TRUE(tree.search(20, rid));
    EXPECT_EQ(rid, RecordId(20, 0));

    EXPECT_TRUE(tree.search(40, rid));
    EXPECT_EQ(rid, RecordId(40, 0));

    const auto result =
        tree.range_scan(10, 50);

    ASSERT_EQ(result.size(), 4);

    EXPECT_EQ(result[0], RecordId(10, 0));
    EXPECT_EQ(result[1], RecordId(20, 0));
    EXPECT_EQ(result[2], RecordId(40, 0));
    EXPECT_EQ(result[3], RecordId(50, 0));
}

TEST(BPlusTreeTest, DeletesUsingLeafMerge) {
    BPlusTree tree;

    for (int key = 10; key <= 40; key += 10) {
        tree.insert(
            key,
            RecordId(key, 0)
        );
    }

    EXPECT_TRUE(tree.remove(10));
    EXPECT_TRUE(tree.remove(20));

    EXPECT_EQ(tree.size(), 2);

    RecordId rid(0, 0);

    EXPECT_FALSE(tree.search(10, rid));
    EXPECT_FALSE(tree.search(20, rid));

    EXPECT_TRUE(tree.search(30, rid));
    EXPECT_EQ(rid, RecordId(30, 0));

    EXPECT_TRUE(tree.search(40, rid));
    EXPECT_EQ(rid, RecordId(40, 0));
}

TEST(BPlusTreeTest, DeletesAllKeys) {
    BPlusTree tree;

    for (int key = 1; key <= 20; ++key) {
        tree.insert(
            key,
            RecordId(key, 0)
        );
    }

    for (int key = 1; key <= 20; ++key) {
        EXPECT_TRUE(tree.remove(key));
    }

    EXPECT_EQ(tree.size(), 0);

    RecordId rid(0, 0);

    for (int key = 1; key <= 20; ++key) {
        EXPECT_FALSE(tree.search(key, rid));
    }
}

TEST(BPlusTreeTest, ShrinksRootAfterDeletingEverything) {
    BPlusTree tree;

    tree.insert(10, RecordId(0, 0));
    tree.insert(20, RecordId(0, 1));
    tree.insert(30, RecordId(0, 2));
    tree.insert(40, RecordId(0, 3));
    tree.insert(50, RecordId(0, 4));

    EXPECT_TRUE(tree.remove(10));
    EXPECT_TRUE(tree.remove(20));
    EXPECT_TRUE(tree.remove(30));
    EXPECT_TRUE(tree.remove(40));
    EXPECT_TRUE(tree.remove(50));

    RecordId rid(0, 0);

    EXPECT_FALSE(tree.search(10, rid));
    EXPECT_FALSE(tree.search(20, rid));
    EXPECT_FALSE(tree.search(30, rid));
    EXPECT_FALSE(tree.search(40, rid));
    EXPECT_FALSE(tree.search(50, rid));

    EXPECT_EQ(tree.size(), 0);
    EXPECT_TRUE(tree.range_scan(0, 100).empty());
}

TEST(BPlusTreeTest, SupportsDuplicateKeys) {
    BPlusTree tree;

    const RecordId first(0, 1);
    const RecordId second(0, 2);
    const RecordId third(1, 0);

    tree.insert(10, first);
    tree.insert(10, second);
    tree.insert(10, third);

    EXPECT_EQ(tree.size(), 3u);

    const std::vector<RecordId> results =
        tree.range_scan(10, 10);

    EXPECT_EQ(results.size(), 3u);
    EXPECT_EQ(results[0], first);
    EXPECT_EQ(results[1], second);
    EXPECT_EQ(results[2], third);
}

TEST(BPlusTreeTest, RemovesSpecificDuplicateKey) {
    BPlusTree tree;

    const RecordId first(0, 1);
    const RecordId second(0, 2);
    const RecordId third(1, 0);

    tree.insert(10, first);
    tree.insert(10, second);
    tree.insert(10, third);

    EXPECT_TRUE(
        tree.remove(10, second)
    );

    EXPECT_EQ(tree.size(), 2u);

    const std::vector<RecordId> results =
        tree.range_scan(10, 10);

    EXPECT_EQ(results.size(), 2u);
    EXPECT_EQ(results[0], first);
    EXPECT_EQ(results[1], third);
}

TEST(BPlusTreeTest, RemovingMissingRecordDoesNothing) {
    BPlusTree tree;

    const RecordId existing(0, 1);
    const RecordId missing(0, 99);

    tree.insert(10, existing);

    EXPECT_FALSE(
        tree.remove(10, missing)
    );

    EXPECT_EQ(tree.size(), 1u);

    RecordId result(0, 0);

    EXPECT_TRUE(
        tree.search(10, result)
    );

    EXPECT_EQ(result, existing);
}

TEST(BPlusTreeTest, RemovesDuplicateKeysAcrossLeaves) {
    BPlusTree tree;

    const RecordId first(0, 1);
    const RecordId second(0, 2);
    const RecordId third(0, 3);
    const RecordId fourth(0, 4);
    const RecordId fifth(0, 5);

    tree.insert(10, first);
    tree.insert(10, second);
    tree.insert(10, third);
    tree.insert(10, fourth);
    tree.insert(10, fifth);

    EXPECT_EQ(tree.size(), 5u);

    EXPECT_TRUE(tree.remove(10, third));
    EXPECT_TRUE(tree.remove(10, fifth));

    const std::vector<RecordId> results =
        tree.range_scan(10, 10);

    EXPECT_EQ(results.size(), 3u);

    EXPECT_EQ(results[0], first);
    EXPECT_EQ(results[1], second);
    EXPECT_EQ(results[2], fourth);
}

} // namespace flashdb