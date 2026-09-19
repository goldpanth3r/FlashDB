#include "b_plus_tree_node.h"

#include <algorithm>
#include <stdexcept>

namespace flashdb {

// Create a node and record whether it is a leaf.
BPlusTreeNode::BPlusTreeNode(
    bool is_leaf)
    : is_leaf_(is_leaf),
      parent_(static_cast<std::size_t>(-1)),
      next_leaf_(static_cast<std::size_t>(-1)) {
}

// Return whether this node stores records.
bool BPlusTreeNode::is_leaf() const {
    return is_leaf_;
}

// Return the sorted keys stored in the node.
const std::vector<int>&
BPlusTreeNode::keys() const {
    return keys_;
}

// Return the RecordIds stored by a leaf.
const std::vector<RecordId>&
BPlusTreeNode::record_ids() const {
    return record_ids_;
}

// Return the child identifiers stored by an internal node.
const std::vector<std::size_t>&
BPlusTreeNode::children() const {
    return children_;
}

// Insert a key and RecordId into the correct sorted position.
void BPlusTreeNode::insert_leaf_entry(
    int key,
    const RecordId& rid) {

    if (!is_leaf_) {
        throw std::runtime_error(
            "BPlusTreeNode: cannot insert leaf entry into internal node"
        );
    }

    // upper_bound places duplicates after existing equal keys.
    auto position = std::upper_bound(
        keys_.begin(),
        keys_.end(),
        key
    );

    const std::size_t index =
        static_cast<std::size_t>(
            position - keys_.begin()
        );

    keys_.insert(
        position,
        key
    );

    record_ids_.insert(
        record_ids_.begin() + index,
        rid
    );
}

// Remove one entry from a leaf node.
bool BPlusTreeNode::remove_leaf_entry(
    int key) {

    if (!is_leaf_) {
        throw std::runtime_error(
            "BPlusTreeNode: cannot remove leaf entry from internal node"
        );
    }

    auto it =
        std::lower_bound(
            keys_.begin(),
            keys_.end(),
            key
        );

    if (it == keys_.end() ||
        *it != key) {
        return false;
    }

    const std::size_t index =
        static_cast<std::size_t>(
            it - keys_.begin()
        );

    keys_.erase(it);

    record_ids_.erase(
        record_ids_.begin() + index
    );

    return true;
}

// Remove the exact key and record ID pair from a leaf.
bool BPlusTreeNode::remove_leaf_entry(
    int key,
    const RecordId& rid) {

    if (!is_leaf_) {
        return false;
    }

    for (std::size_t i = 0;
         i < keys_.size();
         ++i) {

        if (keys_[i] == key &&
            record_ids_[i] == rid) {

            keys_.erase(
                keys_.begin() + i
            );

            record_ids_.erase(
                record_ids_.begin() + i
            );

            return true;
        }
    }

    return false;
}

// Change one separator key in an internal node.
void BPlusTreeNode::set_key(
    std::size_t index,
    int key) {

    if (is_leaf_) {
        throw std::runtime_error(
            "BPlusTreeNode: cannot set internal key in leaf"
        );
    }

    if (index >= keys_.size()) {
        throw std::out_of_range(
            "BPlusTreeNode: invalid key index"
        );
    }

    keys_[index] = key;
}

// Insert an internal separator and its child.
void BPlusTreeNode::insert_internal_entry(
    int key,
    std::size_t child) {

    if (is_leaf_) {
        throw std::runtime_error(
            "BPlusTreeNode: cannot insert internal entry into leaf"
        );
    }

    auto position = std::lower_bound(
        keys_.begin(),
        keys_.end(),
        key
    );

    const std::size_t index =
        static_cast<std::size_t>(
            position - keys_.begin()
        );

    keys_.insert(
        position,
        key
    );

    children_.insert(
        children_.begin() + index + 1,
        child
    );
}

// Remove one separator key and the child to its right.
void BPlusTreeNode::remove_internal_entry(
    std::size_t key_index) {

    if (is_leaf_) {
        throw std::runtime_error(
            "BPlusTreeNode: cannot remove internal entry from leaf"
        );
    }

    if (key_index >= keys_.size()) {
        throw std::out_of_range(
            "BPlusTreeNode: invalid internal key index"
        );
    }

    keys_.erase(
        keys_.begin() + key_index
    );

    children_.erase(
        children_.begin() + key_index + 1
    );
}

// Add a separator and child to the beginning of an internal node.
void BPlusTreeNode::prepend_internal_entry(
    int key,
    std::size_t child) {

    if (is_leaf_) {
        throw std::runtime_error(
            "BPlusTreeNode: cannot prepend internal entry to leaf"
        );
    }

    keys_.insert(
        keys_.begin(),
        key
    );

    children_.insert(
        children_.begin(),
        child
    );
}

// Add a separator key and child to the end of an internal node.
void BPlusTreeNode::append_internal_entry(
    int key,
    std::size_t child) {

    if (is_leaf_) {
        throw std::runtime_error(
            "BPlusTreeNode: cannot append internal entry to leaf"
        );
    }

    keys_.push_back(key);
    children_.push_back(child);
}

// Return the first child of an internal node.
std::size_t BPlusTreeNode::first_child() const {

    if (is_leaf_) {
        throw std::runtime_error(
            "BPlusTreeNode: leaf has no child nodes"
        );
    }

    if (children_.empty()) {
        throw std::runtime_error(
            "BPlusTreeNode: internal node has no children"
        );
    }

    return children_.front();
}

// Remove and return the first child of an internal node.
std::size_t BPlusTreeNode::remove_first_child() {

    if (is_leaf_) {
        throw std::runtime_error(
            "BPlusTreeNode: leaf has no child nodes"
        );
    }

    if (children_.empty()) {
        throw std::runtime_error(
            "BPlusTreeNode: internal node has no children"
        );
    }

    const std::size_t child =
        children_.front();

    children_.erase(
        children_.begin()
    );

    return child;
}

// Return the last child of an internal node.
std::size_t BPlusTreeNode::last_child() const {

    if (is_leaf_) {
        throw std::runtime_error(
            "BPlusTreeNode: leaf has no child nodes"
        );
    }

    if (children_.empty()) {
        throw std::runtime_error(
            "BPlusTreeNode: internal node has no children"
        );
    }

    return children_.back();
}

// Remove and return the last child of an internal node.
std::size_t BPlusTreeNode::remove_last_child() {

    if (is_leaf_) {
        throw std::runtime_error(
            "BPlusTreeNode: leaf has no child nodes"
        );
    }

    if (children_.empty()) {
        throw std::runtime_error(
            "BPlusTreeNode: internal node has no children"
        );
    }

    const std::size_t child =
        children_.back();

    children_.pop_back();

    return child;
}

void BPlusTreeNode::remove_last_key() {
    keys_.pop_back();
}

void BPlusTreeNode::remove_first_key() {
    keys_.erase(keys_.begin());
}

// Return how many separator/data keys the node contains.
std::size_t BPlusTreeNode::key_count() const {
    return keys_.size();
}

// Establish the leftmost child before separator keys are inserted.
void BPlusTreeNode::set_first_child(
    std::size_t child) {

    if (is_leaf_) {
        throw std::runtime_error(
            "BPlusTreeNode: leaf node cannot have children"
        );
    }

    if (!children_.empty()) {
        throw std::runtime_error(
            "BPlusTreeNode: first child is already set"
        );
    }

    children_.push_back(child);
}

// Return the parent node identifier.
std::size_t BPlusTreeNode::parent() const {
    return parent_;
}

// Set the parent node identifier.
void BPlusTreeNode::set_parent(
    std::size_t parent) {

    parent_ = parent;
}

// Return the next leaf node identifier.
std::size_t BPlusTreeNode::next_leaf() const {
    return next_leaf_;
}

// Link this leaf to the next leaf.
void BPlusTreeNode::set_next_leaf(
    std::size_t next_leaf) {

    next_leaf_ = next_leaf;
}

// Create the right half of a leaf during a split.
BPlusTreeNode BPlusTreeNode::split_leaf(
    std::size_t split_index) const {

    if (!is_leaf_) {
        throw std::runtime_error(
            "BPlusTreeNode: cannot split internal node as leaf"
        );
    }

    if (split_index == 0 ||
        split_index >= keys_.size()) {
        throw std::out_of_range(
            "BPlusTreeNode: invalid leaf split position"
        );
    }

    BPlusTreeNode right(true);

    for (std::size_t i = split_index;
         i < keys_.size();
         ++i) {

        right.keys_.push_back(keys_[i]);
        right.record_ids_.push_back(
            record_ids_[i]
        );
    }

    right.parent_ = parent_;
    right.next_leaf_ = next_leaf_;

    return right;
}

// Split the internal node and keep the left half in this node.
BPlusTreeNode BPlusTreeNode::split_internal(
    std::size_t split_index) {

    if (is_leaf_) {
        throw std::runtime_error(
            "BPlusTreeNode: cannot split leaf as internal"
        );
    }

    if (split_index == 0 ||
        split_index >= keys_.size()) {
        throw std::out_of_range(
            "BPlusTreeNode: invalid internal split position"
        );
    }

    if (children_.size() != keys_.size() + 1) {
        throw std::runtime_error(
            "BPlusTreeNode: invalid internal node"
        );
    }

    BPlusTreeNode right(false);

    // The separator key moves up to the parent.
    // It is therefore not stored in either child.
    for (std::size_t i = split_index + 1;
         i < keys_.size();
         ++i) {

        right.keys_.push_back(keys_[i]);
    }

    // The right node gets the children after the separator.
    for (std::size_t i = split_index + 1;
         i < children_.size();
         ++i) {

        right.children_.push_back(
            children_[i]
        );
    }

    // Keep the left side in the original node.
    keys_.erase(
        keys_.begin() + split_index,
        keys_.end()
    );

    children_.erase(
        children_.begin() + split_index + 1,
        children_.end()
    );

    right.parent_ = parent_;

    return right;
}

// Split the leaf and keep the left half in this node.
BPlusTreeNode BPlusTreeNode::split_leaf(
    std::size_t split_index) {

    if (!is_leaf_) {
        throw std::runtime_error(
            "BPlusTreeNode: cannot split internal node as leaf"
        );
    }

    if (split_index == 0 ||
        split_index >= keys_.size()) {
        throw std::out_of_range(
            "BPlusTreeNode: invalid leaf split position"
        );
    }

    BPlusTreeNode right(true);

    for (std::size_t i = split_index;
         i < keys_.size();
         ++i) {

        right.keys_.push_back(keys_[i]);
        right.record_ids_.push_back(
            record_ids_[i]
        );
    }

    keys_.erase(
        keys_.begin() + split_index,
        keys_.end()
    );

    record_ids_.erase(
        record_ids_.begin() + split_index,
        record_ids_.end()
    );

    right.parent_ = parent_;
    right.next_leaf_ = next_leaf_;

    return right;
}

} // namespace flashdb