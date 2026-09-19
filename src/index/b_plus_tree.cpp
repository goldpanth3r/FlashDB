#include "b_plus_tree.h"

#include <algorithm>
#include <stdexcept>

namespace flashdb {

// Create an empty tree with one leaf root.
BPlusTree::BPlusTree()
    : root_id_(0),
      size_(0) {

    create_node(true);
}

// Create a node and return its identifier.
std::size_t BPlusTree::create_node(bool is_leaf) {
    const std::size_t node_id = nodes_.size();

    NodeEntry entry;
    entry.node =
        std::make_unique<BPlusTreeNode>(is_leaf);

    nodes_.push_back(std::move(entry));

    return node_id;
}

// Find the leaf where a key belongs.
std::size_t BPlusTree::find_leaf(int key) const {
    std::size_t current_id = root_id_;

    while (!nodes_[current_id].node->is_leaf()) {
        const auto& node =
            *nodes_[current_id].node;

        const auto& keys = node.keys();
        const auto& children = node.children();

        std::size_t child_index = 0;

        while (child_index < keys.size() &&
               key >= keys[child_index]) {
            ++child_index;
        }

        if (child_index >= children.size()) {
            throw std::runtime_error(
                "BPlusTree: invalid child index"
            );
        }

        current_id = children[child_index];
    }

    return current_id;
}

// Find the first leaf that can contain the given key.
std::size_t BPlusTree::find_first_leaf(int key) const {

    std::size_t current_id = root_id_;

    while (!nodes_[current_id].node->is_leaf()) {

        const auto& node =
            *nodes_[current_id].node;

        const auto& keys =
            node.keys();

        const auto& children =
            node.children();

        std::size_t child_index = 0;

        // For range scans, equal keys must go to the left.
        while (child_index < keys.size() &&
               key > keys[child_index]) {

            ++child_index;
        }

        if (child_index >= children.size()) {
            throw std::runtime_error(
                "BPlusTree: invalid child index"
            );
        }

        current_id =
            children[child_index];
    }

    return current_id;
}

// Insert an entry into a leaf node.
void BPlusTree::insert_into_leaf(
    std::size_t leaf_id,
    int key,
    const RecordId& rid) {

    nodes_[leaf_id].node->insert_leaf_entry(
        key,
        rid
    );
}

// Insert a key and split the leaf when it becomes full.
void BPlusTree::insert(
    int key,
    const RecordId& rid) {

    const std::size_t leaf_id =
        find_leaf(key);

    insert_into_leaf(
        leaf_id,
        key,
        rid
    );

    ++size_;

    if (nodes_[leaf_id].node->key_count() >
        MAX_KEYS) {

        split_leaf(leaf_id);
    }
}

// Split a leaf into two nodes and update its parent.
void BPlusTree::split_leaf(
    std::size_t leaf_id) {

    auto& left =
        *nodes_[leaf_id].node;

    const std::size_t split_index =
        left.key_count() / 2;

    BPlusTreeNode right =
        left.split_leaf(split_index);

    const std::size_t old_next =
        right.next_leaf();

    const std::size_t right_id =
        create_node(true);

    *nodes_[right_id].node =
        std::move(right);

    nodes_[right_id].node->set_next_leaf(
        old_next
    );

    left.set_next_leaf(right_id);

    const int separator =
        nodes_[right_id].node->keys().front();

    insert_into_parent(
        leaf_id,
        separator,
        right_id
    );
}

// Add the new right node to the parent of the split node.
void BPlusTree::insert_into_parent(
    std::size_t left_id,
    int separator,
    std::size_t right_id) {

    auto& left =
        *nodes_[left_id].node;

    const std::size_t parent_id =
        left.parent();

    // The root has no parent, so create a new root.
    if (parent_id ==
        static_cast<std::size_t>(-1)) {

        const std::size_t new_root_id =
            create_node(false);

        auto& root =
            *nodes_[new_root_id].node;

        root.set_first_child(left_id);

        root.insert_internal_entry(
            separator,
            right_id
        );

        nodes_[left_id].node->set_parent(
            new_root_id
        );

        nodes_[right_id].node->set_parent(
            new_root_id
        );

        root_id_ = new_root_id;

        return;
    }

    auto& parent =
        *nodes_[parent_id].node;

    parent.insert_internal_entry(
        separator,
        right_id
    );

    nodes_[right_id].node->set_parent(
        parent_id
    );

    if (parent.key_count() > MAX_KEYS) {
        split_internal(parent_id);
    }
}

// Split an internal node and push its separator into the parent.
void BPlusTree::split_internal(
    std::size_t node_id) {

    auto& left =
        *nodes_[node_id].node;

    const std::size_t split_index =
        left.key_count() / 2;

    const int separator =
        left.keys()[split_index];

    BPlusTreeNode right =
        left.split_internal(split_index);

    const std::size_t right_id =
        create_node(false);

    *nodes_[right_id].node =
        std::move(right);

    nodes_[right_id].node->set_parent(
        left.parent()
    );

    // The separator itself moves up to the parent.
    insert_into_parent(
        node_id,
        separator,
        right_id
    );

    // Update the parent of every child moved
    // into the new right internal node.
    for (const std::size_t child_id :
         nodes_[right_id].node->children()) {

        nodes_[child_id].node->set_parent(
            right_id
        );
    }
}

// Search for an exact key.
bool BPlusTree::search(
    int key,
    RecordId& rid) const {

    const std::size_t leaf_id =
        find_leaf(key);

    const auto& node =
        *nodes_[leaf_id].node;

    const auto& keys = node.keys();
    const auto& records = node.record_ids();

    auto it =
        std::lower_bound(
            keys.begin(),
            keys.end(),
            key
        );

    if (it == keys.end() ||
        *it != key) {

        return false;
    }

    const std::size_t index =
        static_cast<std::size_t>(
            it - keys.begin()
        );

    rid = records[index];

    return true;
}

// Remove a key directly from a leaf node.
bool BPlusTree::remove_from_leaf(
    std::size_t leaf_id,
    int key) {

    return nodes_[leaf_id].node->remove_leaf_entry(
        key
    );
}

// Remove a key and rebalance its leaf if necessary.
bool BPlusTree::remove(int key) {

    const std::size_t leaf_id =
        find_leaf(key);

    if (!remove_from_leaf(
            leaf_id,
            key)) {

        return false;
    }

    --size_;

    if (nodes_[leaf_id].node->key_count() == 0 &&
        leaf_id != root_id_) {

        rebalance_leaf(leaf_id);
    }

    return true;
}

// Remove one specific record from an indexed key.
bool BPlusTree::remove(
    int key,
    const RecordId& rid) {

    // Duplicates may exist in several leaves, so start
    // from the first leaf containing this key.
    std::size_t leaf_id =
        find_first_leaf(key);

    while (true) {

        auto& leaf =
            *nodes_[leaf_id].node;

        // Stop once keys have moved beyond the target key.
        if (!leaf.keys().empty() &&
            leaf.keys().front() > key) {

            return false;
        }

        if (leaf.remove_leaf_entry(
                key,
                rid)) {

            --size_;

            if (leaf.key_count() == 0 &&
                leaf_id != root_id_) {

                rebalance_leaf(leaf_id);
            }

            return true;
        }

        const auto& keys =
            leaf.keys();

        // If this leaf contains keys greater than the
        // requested key, the record cannot appear later.
        if (!keys.empty() &&
            keys.back() > key) {

            return false;
        }

        const std::size_t next =
            leaf.next_leaf();

        if (next ==
            static_cast<std::size_t>(-1)) {

            return false;
        }

        leaf_id = next;
    }
}

// Return records inside an inclusive key range.
std::vector<RecordId> BPlusTree::range_scan(
    int start_key,
    int end_key) const {

    std::vector<RecordId> result;

    if (start_key > end_key) {
        return result;
    }

    // Start from the leftmost leaf that can contain start_key.
    std::size_t leaf_id =
        find_first_leaf(start_key);

    while (true) {

        const auto& node =
            *nodes_[leaf_id].node;

        const auto& keys =
            node.keys();

        const auto& records =
            node.record_ids();

        for (std::size_t i = 0;
             i < keys.size();
             ++i) {

            if (keys[i] < start_key) {
                continue;
            }

            if (keys[i] > end_key) {
                return result;
            }

            result.push_back(
                records[i]
            );
        }

        const std::size_t next =
            node.next_leaf();

        if (next ==
            static_cast<std::size_t>(-1)) {

            break;
        }

        leaf_id = next;
    }

    return result;
}

// Return the number of keys in the tree.
std::size_t BPlusTree::size() const {
    return size_;
}

// Borrow from a sibling or merge leaves when a leaf becomes empty.
void BPlusTree::rebalance_leaf(
    std::size_t leaf_id) {

    auto& leaf =
        *nodes_[leaf_id].node;

    const std::size_t parent_id =
        leaf.parent();

    if (parent_id ==
        static_cast<std::size_t>(-1)) {

        return;
    }

    auto& parent =
        *nodes_[parent_id].node;

    const auto& children =
        parent.children();

    std::size_t child_index = 0;

    while (child_index < children.size() &&
           children[child_index] != leaf_id) {

        ++child_index;
    }

    if (child_index >= children.size()) {
        throw std::runtime_error(
            "BPlusTree: leaf not found in parent"
        );
    }

    // Try the left sibling first.
    if (child_index > 0) {

        const std::size_t left_id =
            children[child_index - 1];

        auto& left =
            *nodes_[left_id].node;

        if (left.key_count() > 1) {

            const int key =
                left.keys().back();

            const RecordId rid =
                left.record_ids().back();

            left.remove_leaf_entry(key);

            leaf.insert_leaf_entry(
                key,
                rid
            );

            parent.set_key(
                child_index - 1,
                leaf.keys().front()
            );

            return;
        }
    }

    // Try the right sibling.
    if (child_index + 1 < children.size()) {

        const std::size_t right_id =
            children[child_index + 1];

        auto& right =
            *nodes_[right_id].node;

        if (right.key_count() > 1) {

            const int key =
                right.keys().front();

            const RecordId rid =
                right.record_ids().front();

            right.remove_leaf_entry(key);

            leaf.insert_leaf_entry(
                key,
                rid
            );

            parent.set_key(
                child_index,
                right.keys().front()
            );

            return;
        }
    }

    // No sibling can spare an entry.
    // Merge this leaf into the left sibling.
    if (child_index > 0) {

        const std::size_t left_id =
            children[child_index - 1];

        auto& left =
            *nodes_[left_id].node;

        left.set_next_leaf(
            leaf.next_leaf()
        );

       parent.remove_internal_entry(
            child_index - 1
        );

        nodes_[leaf_id].node.reset();

        if (parent_id != root_id_ &&
            parent.key_count() == 0) {

            rebalance_internal(parent_id);
        }

        return;
    }

    // Otherwise merge the right sibling into this leaf.
    if (child_index + 1 < children.size()) {

        const std::size_t right_id =
            children[child_index + 1];

        auto& right =
            *nodes_[right_id].node;

        for (std::size_t i = 0;
             i < right.key_count();
             ++i) {

            leaf.insert_leaf_entry(
                right.keys()[i],
                right.record_ids()[i]
            );
        }

        leaf.set_next_leaf(
            right.next_leaf()
        );

        parent.remove_internal_entry(
            child_index
        );

        nodes_[right_id].node.reset();

        if (parent_id != root_id_ &&
            parent.key_count() == 0) {

            rebalance_internal(parent_id);
        }
    }
}

// Borrow one separator and child from a sibling.
// Restore an underfull internal node by borrowing from a sibling.
void BPlusTree::rebalance_internal(
    std::size_t node_id) {

    auto& node =
        *nodes_[node_id].node;

    const std::size_t parent_id =
        node.parent();

    if (parent_id ==
        static_cast<std::size_t>(-1)) {

        return;
    }

    auto& parent =
        *nodes_[parent_id].node;

    const auto& children =
        parent.children();

    std::size_t index = 0;

    while (index < children.size() &&
           children[index] != node_id) {

        ++index;
    }

    if (index >= children.size()) {
        throw std::runtime_error(
            "BPlusTree: internal node not found in parent"
        );
    }

    // Borrow from the left sibling.
    if (index > 0) {

        const std::size_t left_id =
            children[index - 1];

        auto& left =
            *nodes_[left_id].node;

        if (left.key_count() > 1) {

            const int separator =
                parent.keys()[index - 1];

            const int new_separator =
                left.keys().back();

            const std::size_t borrowed_child =
                left.remove_last_child();

            left.remove_last_key();

            node.prepend_internal_entry(
                separator,
                borrowed_child
            );

            parent.set_key(
                index - 1,
                new_separator
            );

            nodes_[borrowed_child].node->set_parent(
                node_id
            );

            return;
        }
    }

    // Borrow from the right sibling.
    if (index + 1 < children.size()) {

        const std::size_t right_id =
            children[index + 1];

        auto& right =
            *nodes_[right_id].node;

        if (right.key_count() > 1) {

            const int separator =
                parent.keys()[index];

            const int new_separator =
                right.keys().front();

            const std::size_t borrowed_child =
                right.remove_first_child();

            right.remove_first_key();

            node.append_internal_entry(
                separator,
                borrowed_child
            );

            parent.set_key(
                index,
                new_separator
            );

            nodes_[borrowed_child].node->set_parent(
                node_id
            );

            return;
        }
    }

    // Merge with left sibling.
    if (index > 0) {

        const std::size_t left_id =
            children[index - 1];

        merge_internal(
            left_id,
            node_id,
            parent.keys()[index - 1]
        );

        parent.remove_internal_entry(
            index - 1
        );

        if (parent_id == root_id_) {
            shrink_root();
        }
        else if (parent.key_count() == 0) {
            rebalance_internal(parent_id);
        }

        return;
    }

    // Merge with right sibling.
    if (index + 1 < children.size()) {

        const std::size_t right_id =
            children[index + 1];

        merge_internal(
            node_id,
            right_id,
            parent.keys()[index]
        );

        parent.remove_internal_entry(index);

        if (parent_id == root_id_) {
            shrink_root();
        }
        else if (parent.key_count() == 0) {
            rebalance_internal(parent_id);
        }
    }
}

// Replace an empty internal root with its only child.
void BPlusTree::shrink_root() {

    auto& root =
        *nodes_[root_id_].node;

    if (root.is_leaf()) {
        return;
    }

    if (root.key_count() != 0) {
        return;
    }

    if (root.children().empty()) {
        return;
    }

    const std::size_t new_root =
        root.children().front();

    nodes_[new_root].node->set_parent(
        static_cast<std::size_t>(-1)
    );

    nodes_[root_id_].node.reset();

    root_id_ = new_root;
}

// Merge the right internal node into the left internal node.
void BPlusTree::merge_internal(
    std::size_t left_id,
    std::size_t right_id,
    int separator_key) {

    auto& left =
        *nodes_[left_id].node;

    auto& right =
        *nodes_[right_id].node;

    // Bring the parent separator down.
    left.append_internal_entry(
        separator_key,
        right.first_child()
    );

    nodes_[right.first_child()].node->set_parent(
        left_id
    );

    // Copy remaining keys and children.
    for (std::size_t i = 0;
         i < right.key_count();
         ++i) {

        left.append_internal_entry(
            right.keys()[i],
            right.children()[i + 1]
        );

        nodes_[right.children()[i + 1]].node->set_parent(
            left_id
        );
    }

    nodes_[right_id].node.reset();
}

} // namespace flashdb