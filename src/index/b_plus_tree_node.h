#pragma once

#include <cstddef>
#include <vector>

#include "record/rid.h"

namespace flashdb {

/**
 * BPlusTreeNode represents one node in the B+ Tree.
 *
 * Leaf nodes store keys and RecordIds.
 * Internal nodes store separator keys and child node identifiers.
 */
class BPlusTreeNode {
public:
    // Create a new node.
    explicit BPlusTreeNode(bool is_leaf);

    // Check whether this node is a leaf.
    bool is_leaf() const;

    // Return the keys stored in this node.
    const std::vector<int>& keys() const;

    // Return the RecordIds stored in a leaf node.
    const std::vector<RecordId>& record_ids() const;

    // Return the child node identifiers stored in an internal node.
    const std::vector<std::size_t>& children() const;

    // Set the first child of an internal node.
    void set_first_child(std::size_t child);

    // Insert a key and RecordId into a leaf node.
    void insert_leaf_entry(
        int key,
        const RecordId& rid
    );
    
    // Remove a key and its RecordId from a leaf node.
    bool remove_leaf_entry(int key);

    bool remove_leaf_entry(
        int key,
        const RecordId& rid
    );

    // Change one separator key in an internal node.
    void set_key(
        std::size_t index,
        int key
    );


    // Insert a separator key and child into an internal node.
    void insert_internal_entry(
        int key,
        std::size_t child
    );

    // Remove one separator key and its right child.
    void remove_internal_entry(
        std::size_t key_index
    );

    // Add a child and separator to the beginning of an internal node.
    void prepend_internal_entry(
        int key,
        std::size_t child
    );

    // Add a separator key and child to the end of an internal node.
    void append_internal_entry(
        int key,
        std::size_t child
    );

    void remove_last_key();
    void remove_first_key();

    // Return the first child of an internal node.
    std::size_t first_child() const;

    // Remove the first child from an internal node.
    std::size_t remove_first_child();

    // Return the last child of an internal node.
    std::size_t last_child() const;

    // Remove the last child from an internal node.
    std::size_t remove_last_child();
    
    // Return the number of keys in this node.
    std::size_t key_count() const;

    // Return the parent node identifier.
    std::size_t parent() const;

    // Set the parent node identifier.
    void set_parent(std::size_t parent);

    // Return the next leaf node identifier.
    std::size_t next_leaf() const;

    // Set the next leaf node identifier.
    void set_next_leaf(std::size_t next_leaf);

    // Create the right half of a leaf during a split.
    BPlusTreeNode split_leaf(
        std::size_t split_index
    ) const;

    // Create the right half of an internal node during a split.
    BPlusTreeNode split_internal(
        std::size_t split_index
    );

    // Split this leaf and keep only the left half.
    BPlusTreeNode split_leaf(
        std::size_t split_index
    );

private:
    bool is_leaf_;

    std::vector<int> keys_;

    std::vector<RecordId> record_ids_;

    std::vector<std::size_t> children_;

    std::size_t parent_;

    std::size_t next_leaf_;
};

} // namespace flashdb