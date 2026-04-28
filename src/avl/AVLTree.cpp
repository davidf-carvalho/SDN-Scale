#include "AVLTree.hpp"
#include <algorithm>

template <typename T>
AVLTree<T>::AVLTree() : root(nullptr), count(0) {}

template <typename T>
AVLTree<T>::~AVLTree() { destroy(root); }

template <typename T>
void AVLTree<T>::destroy(Node* n) {
    if (!n) return;
    destroy(n->left);
    destroy(n->right);
    delete n;
}

template <typename T>
int AVLTree<T>::height(Node* n) const {
    return n ? n->height : 0;
}

template <typename T>
int AVLTree<T>::balanceFactor(Node* n) const {
    return n ? height(n->left) - height(n->right) : 0;
}

template <typename T>
typename AVLTree<T>::Node* AVLTree<T>::rotateRight(Node* y) {
    Node* x = y->left;
    Node* t = x->right;
    x->right = y;
    y->left = t;
    y->height = 1 + std::max(height(y->left), height(y->right));
    x->height = 1 + std::max(height(x->left), height(x->right));
    return x;
}

template <typename T>
typename AVLTree<T>::Node* AVLTree<T>::rotateLeft(Node* x) {
    Node* y = x->right;
    Node* t = y->left;
    y->left = x;
    x->right = t;
    x->height = 1 + std::max(height(x->left), height(x->right));
    y->height = 1 + std::max(height(y->left), height(y->right));
    return y;
}

template <typename T>
typename AVLTree<T>::Node* AVLTree<T>::balance(Node* n) {
    n->height = 1 + std::max(height(n->left), height(n->right));
    int bf = balanceFactor(n);
    if (bf > 1 && balanceFactor(n->left) >= 0)  return rotateRight(n);
    if (bf > 1 && balanceFactor(n->left) < 0)   { n->left = rotateLeft(n->left);  return rotateRight(n); }
    if (bf < -1 && balanceFactor(n->right) <= 0) return rotateLeft(n);
    if (bf < -1 && balanceFactor(n->right) > 0)  { n->right = rotateRight(n->right); return rotateLeft(n); }
    return n;
}

template <typename T>
typename AVLTree<T>::Node* AVLTree<T>::insert(Node* n, const T& key) {
    if (!n) { ++count; return new Node(key); }
    if (key < n->key)      n->left  = insert(n->left,  key);
    else if (key > n->key) n->right = insert(n->right, key);
    return balance(n);
}

template <typename T>
typename AVLTree<T>::Node* AVLTree<T>::minNode(Node* n) const {
    return n->left ? minNode(n->left) : n;
}

template <typename T>
typename AVLTree<T>::Node* AVLTree<T>::remove(Node* n, const T& key) {
    if (!n) return nullptr;
    if (key < n->key)      n->left  = remove(n->left,  key);
    else if (key > n->key) n->right = remove(n->right, key);
    else {
        if (!n->left || !n->right) {
            Node* tmp = n->left ? n->left : n->right;
            delete n;
            --count;
            return tmp;
        }
        Node* mn = minNode(n->right);
        n->key = mn->key;
        n->right = remove(n->right, mn->key);
    }
    return balance(n);
}

template <typename T>
bool AVLTree<T>::search(Node* n, const T& key) const {
    if (!n) return false;
    if (key == n->key) return true;
    return key < n->key ? search(n->left, key) : search(n->right, key);
}

template <typename T>
void AVLTree<T>::insert(const T& key) { root = insert(root, key); }

template <typename T>
void AVLTree<T>::remove(const T& key) { root = remove(root, key); }

template <typename T>
bool AVLTree<T>::search(const T& key) const { return search(root, key); }

template <typename T>
int AVLTree<T>::size() const { return count; }
