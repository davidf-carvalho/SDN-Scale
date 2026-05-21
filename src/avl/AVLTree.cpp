#include "AVLTree.hpp"
#include <utility>

namespace sdn {

int AVLTree::node_height(const Node* n) {
    return n ? n->height : 0;
}

int AVLTree::balance_factor(const Node* n) {
    return node_height(n->left) - node_height(n->right);
}

void AVLTree::update_height(Node* n) {
    int lh = node_height(n->left);
    int rh = node_height(n->right);
    n->height = 1 + (lh > rh ? lh : rh);
}

AVLTree::AVLTree(size_t reserve)
    : root_(nullptr), size_(0), rotations_(0)
{
    if (reserve > 0) free_list_.reserve(reserve);
}

AVLTree::~AVLTree() {
    clear();
    for (Node* n : free_list_) delete n;
}

AVLTree::AVLTree(AVLTree&& o)
    : root_(o.root_), size_(o.size_), rotations_(o.rotations_),
      free_list_(std::move(o.free_list_))
{
    o.root_ = nullptr;
    o.size_ = 0;
    o.rotations_ = 0;
}

AVLTree& AVLTree::operator=(AVLTree&& o) {
    if (this != &o) {
        clear();
        for (Node* n : free_list_) delete n;
        root_ = o.root_;
        size_ = o.size_;
        rotations_ = o.rotations_;
        free_list_ = std::move(o.free_list_);
        o.root_ = nullptr;
        o.size_ = 0;
        o.rotations_ = 0;
    }
    return *this;
}

AVLTree::Node* AVLTree::alloc_node(const PacketRule& rule) {
    if (!free_list_.empty()) {
        Node* n = free_list_.back();
        free_list_.pop_back();
        n->rule = rule;
        n->left = nullptr;
        n->right = nullptr;
        n->height = 1;
        return n;
    }
    return new Node(rule);
}

void AVLTree::release_node(Node* n) {
    free_list_.push_back(n);
}

AVLTree::Node* AVLTree::rotate_right(Node* y) {
    Node* x = y->left;
    Node* T2 = x->right;
    x->right = y;
    y->left = T2;
    update_height(y);
    update_height(x);
    ++rotations_;
    return x;
}

AVLTree::Node* AVLTree::rotate_left(Node* x) {
    Node* y = x->right;
    Node* T2 = y->left;
    y->left = x;
    x->right = T2;
    update_height(x);
    update_height(y);
    ++rotations_;
    return y;
}

AVLTree::Node* AVLTree::rebalance(Node* n) {
    update_height(n);
    int fb = balance_factor(n);

    if (fb > 1 && balance_factor(n->left) >= 0) {
        return rotate_right(n);
    }
    if (fb > 1 && balance_factor(n->left) < 0) {
        n->left = rotate_left(n->left);
        return rotate_right(n);
    }
    if (fb < -1 && balance_factor(n->right) <= 0) {
        return rotate_left(n);
    }
    if (fb < -1 && balance_factor(n->right) > 0) {
        n->right = rotate_right(n->right);
        return rotate_left(n);
    }
    return n;
}

AVLTree::Node* AVLTree::insert_rec(Node* n, const PacketRule& rule, bool& inserted) {
    if (!n) {
        inserted = true;
        return alloc_node(rule);
    }

    if (rule.id < n->rule.id) {
        n->left = insert_rec(n->left, rule, inserted);
    } else if (rule.id > n->rule.id) {
        n->right = insert_rec(n->right, rule, inserted);
    } else {
        n->rule = rule;
        inserted = false;
        return n;
    }
    return rebalance(n);
}

void AVLTree::insert(const PacketRule& rule) {
    bool inserted = false;
    root_ = insert_rec(root_, rule, inserted);
    if (inserted) ++size_;
}

const PacketRule* AVLTree::search_rec(const Node* n, uint32_t id) {
    while (n) {
        if (id < n->rule.id) n = n->left;
        else if (id > n->rule.id) n = n->right;
        else return &n->rule;
    }
    return nullptr;
}

const PacketRule* AVLTree::search(uint32_t id) const {
    return search_rec(root_, id);
}

AVLTree::Node* AVLTree::min_node(Node* n) {
    while (n->left) n = n->left;
    return n;
}

AVLTree::Node* AVLTree::remove_rec(Node* n, uint32_t id, bool& removed) {
    if (!n) return nullptr;

    if (id < n->rule.id) {
        n->left = remove_rec(n->left, id, removed);
    } else if (id > n->rule.id) {
        n->right = remove_rec(n->right, id, removed);
    } else {
        removed = true;
        if (!n->left || !n->right) {
            Node* child = n->left ? n->left : n->right;
            release_node(n);
            return child;
        }
        Node* successor = min_node(n->right);
        n->rule = successor->rule;
        n->right = remove_rec(n->right, successor->rule.id, removed);
        removed = true;
    }
    return rebalance(n);
}

bool AVLTree::remove(uint32_t id) {
    bool removed = false;
    root_ = remove_rec(root_, id, removed);
    if (removed) --size_;
    return removed;
}

int AVLTree::height() const {
    return node_height(root_);
}

bool AVLTree::validate_rec(const Node* n, int& height) {
    if (!n) {
        height = 0;
        return true;
    }

    int lh = 0, rh = 0;
    if (!validate_rec(n->left, lh)) return false;
    if (!validate_rec(n->right, rh)) return false;

    int fb = lh - rh;
    if (fb < -1 || fb > 1) return false;

    int expected = 1 + (lh > rh ? lh : rh);
    if (n->height != expected) return false;

    height = expected;
    return true;
}

bool AVLTree::validate() const {
    int h = 0;
    return validate_rec(root_, h);
}

void AVLTree::destroy_rec(Node* n) {
    if (!n) return;
    destroy_rec(n->left);
    destroy_rec(n->right);
    release_node(n);
}

void AVLTree::clear() {
    destroy_rec(root_);
    root_ = nullptr;
    size_ = 0;
}

} // namespace sdn
