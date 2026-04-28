#include "RedBlackTree.hpp"

template <typename T>
RedBlackTree<T>::RedBlackTree() : count(0) {
    nil = new Node(T{});
    nil->color = BLACK;
    nil->left = nil->right = nil->parent = nil;
    root = nil;
}

template <typename T>
RedBlackTree<T>::~RedBlackTree() {
    destroy(root);
    delete nil;
}

template <typename T>
void RedBlackTree<T>::destroy(Node* n) {
    if (n == nil) return;
    destroy(n->left);
    destroy(n->right);
    delete n;
}

template <typename T>
void RedBlackTree<T>::leftRotate(Node* x) {
    Node* y = x->right;
    x->right = y->left;
    if (y->left != nil) y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == nil)       root = y;
    else if (x == x->parent->left) x->parent->left  = y;
    else                           x->parent->right = y;
    y->left = x;
    x->parent = y;
}

template <typename T>
void RedBlackTree<T>::rightRotate(Node* x) {
    Node* y = x->left;
    x->left = y->right;
    if (y->right != nil) y->right->parent = x;
    y->parent = x->parent;
    if (x->parent == nil)        root = y;
    else if (x == x->parent->right) x->parent->right = y;
    else                            x->parent->left  = y;
    y->right = x;
    x->parent = y;
}

template <typename T>
void RedBlackTree<T>::insertFixup(Node* z) {
    while (z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            Node* y = z->parent->parent->right;
            if (y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) { z = z->parent; leftRotate(z); }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rightRotate(z->parent->parent);
            }
        } else {
            Node* y = z->parent->parent->left;
            if (y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) { z = z->parent; rightRotate(z); }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                leftRotate(z->parent->parent);
            }
        }
    }
    root->color = BLACK;
}

template <typename T>
void RedBlackTree<T>::insert(const T& key) {
    Node* z = new Node(key);
    z->left = z->right = z->parent = nil;
    Node* y = nil;
    Node* x = root;
    while (x != nil) {
        y = x;
        x = (z->key < x->key) ? x->left : x->right;
    }
    z->parent = y;
    if (y == nil)          root = z;
    else if (z->key < y->key) y->left  = z;
    else                       y->right = z;
    ++count;
    insertFixup(z);
}

template <typename T>
typename RedBlackTree<T>::Node* RedBlackTree<T>::minimum(Node* x) const {
    while (x->left != nil) x = x->left;
    return x;
}

template <typename T>
void RedBlackTree<T>::transplant(Node* u, Node* v) {
    if (u->parent == nil)          root = v;
    else if (u == u->parent->left) u->parent->left  = v;
    else                           u->parent->right = v;
    v->parent = u->parent;
}

template <typename T>
void RedBlackTree<T>::removeFixup(Node* x) {
    while (x != root && x->color == BLACK) {
        if (x == x->parent->left) {
            Node* w = x->parent->right;
            if (w->color == RED) { w->color = BLACK; x->parent->color = RED; leftRotate(x->parent); w = x->parent->right; }
            if (w->left->color == BLACK && w->right->color == BLACK) { w->color = RED; x = x->parent; }
            else {
                if (w->right->color == BLACK) { w->left->color = BLACK; w->color = RED; rightRotate(w); w = x->parent->right; }
                w->color = x->parent->color; x->parent->color = BLACK; w->right->color = BLACK; leftRotate(x->parent); x = root;
            }
        } else {
            Node* w = x->parent->left;
            if (w->color == RED) { w->color = BLACK; x->parent->color = RED; rightRotate(x->parent); w = x->parent->left; }
            if (w->right->color == BLACK && w->left->color == BLACK) { w->color = RED; x = x->parent; }
            else {
                if (w->left->color == BLACK) { w->right->color = BLACK; w->color = RED; leftRotate(w); w = x->parent->left; }
                w->color = x->parent->color; x->parent->color = BLACK; w->left->color = BLACK; rightRotate(x->parent); x = root;
            }
        }
    }
    x->color = BLACK;
}

template <typename T>
void RedBlackTree<T>::remove(const T& key) {
    Node* z = root;
    while (z != nil && !(z->key == key))
        z = (key < z->key) ? z->left : z->right;
    if (z == nil) return;
    --count;
    Node* y = z;
    Node* x;
    Color origColor = y->color;
    if (z->left == nil)      { x = z->right; transplant(z, z->right); }
    else if (z->right == nil) { x = z->left;  transplant(z, z->left); }
    else {
        y = minimum(z->right);
        origColor = y->color;
        x = y->right;
        if (y->parent == z) x->parent = y;
        else { transplant(y, y->right); y->right = z->right; y->right->parent = y; }
        transplant(z, y);
        y->left = z->left; y->left->parent = y; y->color = z->color;
    }
    delete z;
    if (origColor == BLACK) removeFixup(x);
}

template <typename T>
bool RedBlackTree<T>::search(Node* n, const T& key) const {
    if (n == nil) return false;
    if (n->key == key) return true;
    return key < n->key ? search(n->left, key) : search(n->right, key);
}

template <typename T>
bool RedBlackTree<T>::search(const T& key) const { return search(root, key); }

template <typename T>
int RedBlackTree<T>::size() const { return count; }
