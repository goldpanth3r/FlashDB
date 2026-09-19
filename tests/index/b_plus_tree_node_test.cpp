#include <gtest/gtest.h>

#include "index/b_plus_tree_node.h"

namespace flashdb {

TEST(BPlusTreeNodeTest, LeafStartsEmpty) {
    BPlusTreeNode node(true);

    EXPECT_TRUE(node.is_leaf());
    EXPECT_EQ(node.key_count(), 0);
    EXPECT_TRUE(node.keys().empty());
    EXPECT_TRUE(node.record_ids().empty());
}

TEST(BPlusTreeNodeTest, InternalStartsEmpty) {
    BPlusTreeNode node(false);

    EXPECT_FALSE(node.is_leaf());
    EXPECT_EQ(node.key_count(), 0);
    EXPECT_TRUE(node.keys().empty());
    EXPECT_TRUE(node.children().empty());
}

TEST(BPlusTreeNodeTest, InsertsLeafEntriesInKeyOrder) {
    BPlusTreeNode node(true);

    node.insert_leaf_entry(
        30,
        RecordId(3, 0)
    );

    node.insert_leaf_entry(
        10,
        RecordId(1, 0)
    );

    node.insert_leaf_entry(
        20,
        RecordId(2, 0)
    );

    ASSERT_EQ(node.key_count(), 3);

    EXPECT_EQ(node.keys()[0], 10);
    EXPECT_EQ(node.keys()[1], 20);
    EXPECT_EQ(node.keys()[2], 30);

    EXPECT_EQ(
        node.record_ids()[0],
        RecordId(1, 0)
    );

    EXPECT_EQ(
        node.record_ids()[1],
        RecordId(2, 0)
    );

    EXPECT_EQ(
        node.record_ids()[2],
        RecordId(3, 0)
    );
}

TEST(BPlusTreeNodeTest, InsertsInternalEntries) {
    BPlusTreeNode node(false);

    node.set_first_child(0);

    node.insert_internal_entry(
        20,
        1
    );

    node.insert_internal_entry(
        40,
        2
    );

    ASSERT_EQ(node.key_count(), 2);
    ASSERT_EQ(node.children().size(), 3);

    EXPECT_EQ(node.keys()[0], 20);
    EXPECT_EQ(node.keys()[1], 40);

    EXPECT_EQ(node.children()[1], 1);
    EXPECT_EQ(node.children()[2], 2);
}

TEST(BPlusTreeNodeTest, RejectsLeafEntryInInternalNode) {
    BPlusTreeNode node(false);

    EXPECT_THROW(
        node.insert_leaf_entry(
            10,
            RecordId(1, 0)
        ),
        std::runtime_error
    );
}

TEST(BPlusTreeNodeTest, RejectsInternalEntryInLeafNode) {
    BPlusTreeNode node(true);

    EXPECT_THROW(
        node.insert_internal_entry(
            10,
            1
        ),
        std::runtime_error
    );
}


TEST(BPlusTreeNodeTest, SplitsInternalNode) {
    BPlusTreeNode node(false);

    node.set_first_child(0);

    node.insert_internal_entry(20, 1);
    node.insert_internal_entry(40, 2);
    node.insert_internal_entry(60, 3);
    node.insert_internal_entry(80, 4);

    BPlusTreeNode right =
        node.split_internal(2);

    // Left node keeps keys before the separator.
    ASSERT_EQ(node.key_count(), 2);

    EXPECT_EQ(node.keys()[0], 20);
    EXPECT_EQ(node.keys()[1], 40);

    ASSERT_EQ(node.children().size(), 3);

    EXPECT_EQ(node.children()[0], 0);
    EXPECT_EQ(node.children()[1], 1);
    EXPECT_EQ(node.children()[2], 2);

    // The separator key 60 moves to the parent.
    // The right node starts after it.
    ASSERT_EQ(right.key_count(), 1);

    EXPECT_EQ(right.keys()[0], 80);

    ASSERT_EQ(right.children().size(), 2);

    EXPECT_EQ(right.children()[0], 3);
    EXPECT_EQ(right.children()[1], 4);
}

TEST(BPlusTreeNodeTest, SplitsLeafNode) {
    BPlusTreeNode node(true);

    node.insert_leaf_entry(10, RecordId(1, 0));
    node.insert_leaf_entry(20, RecordId(2, 0));
    node.insert_leaf_entry(30, RecordId(3, 0));
    node.insert_leaf_entry(40, RecordId(4, 0));

    node.set_next_leaf(99);

    BPlusTreeNode right =
        node.split_leaf(2);

    ASSERT_EQ(node.key_count(), 2);
    EXPECT_EQ(node.keys()[0], 10);
    EXPECT_EQ(node.keys()[1], 20);

    ASSERT_EQ(right.key_count(), 2);

    EXPECT_EQ(right.keys()[0], 30);
    EXPECT_EQ(right.keys()[1], 40);

    EXPECT_EQ(
        right.record_ids()[0],
        RecordId(3, 0)
    );

    EXPECT_EQ(
        right.record_ids()[1],
        RecordId(4, 0)
    );

    EXPECT_EQ(right.next_leaf(), 99);
}
} // namespace flashdb