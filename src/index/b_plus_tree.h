#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "b_plus_tree_node.h"
#include "record/rid.h"

namespace flashdb {

/**
 * BPlusTree stores RecordIds ordered by integer keys.
 *
 * The tree starts with one leaf node and grows by splitting
 * full leaf and internal nodes.
 */
class BPlusTree {
public:
    // Create an empty B+ Tree.
    BPlusTree();

    // Insert a key and the record it points to.
    void insert(int key, const RecordId& rid);

    // Find the record for an exact key.
    bool search(int key, RecordId& rid) const;

    // Remove a key from the tree.
    bool remove(int key);

    bool remove(
        int key,
        const RecordId& rid
    );

    // Find all records whose keys are within the given range.
    std::vector<RecordId> range_scan(
        int start_key,
        int end_key
    ) const;

    // Return the number of keys currently stored.
    std::size_t size() const;

private:
    static constexpr std::size_t MAX_KEYS = 3;

    struct NodeEntry {
        std::unique_ptr<BPlusTreeNode> node;
    };

    std::vector<NodeEntry> nodes_;

    std::size_t root_id_;
    std::size_t size_;

    // Create a new tree node and return its identifier.
    std::size_t create_node(bool is_leaf);

    // Find the leaf node that should contain the key.
    std::size_t find_leaf(int key) const;

    std::size_t find_first_leaf(int key) const;

    // Insert into a leaf that still has space.
    void insert_into_leaf(
        std::size_t leaf_id,
        int key,
        const RecordId& rid
    );

    // Split a full leaf node.
    void split_leaf(
        std::size_t leaf_id
    );

    // Add a new child to an internal parent node.
    void insert_into_parent(
        std::size_t left_id,
        int separator,
        std::size_t right_id
    );

    // Split a full internal node.
    void split_internal(
        std::size_t node_id
    );

    // Remove a key directly from a leaf node.
    bool remove_from_leaf(
        std::size_t leaf_id,
        int key
    );

    // Restore the minimum number of entries in an underfull leaf.
    void rebalance_leaf(
        std::size_t leaf_id
    );
    
    // Restore an underfull internal node by borrowing from siblings.
    void rebalance_internal(
        std::size_t node_id
    );

    // Replace an empty root with its only child.
    void shrink_root();

    // Merge two internal nodes.
    void merge_internal(
        std::size_t left_id,
        std::size_t right_id,
        int separator_key
    );

};



} // namespace flashdb