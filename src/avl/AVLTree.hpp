#pragma once
#include <functional>

template <typename T>
class AVLTree {
public:
    AVLTree();
    ~AVLTree();

    void insert(const T& key);
    void remove(const T& key);
    bool search(const T& key) const;
    int size() const;

private:
    struct Node {
        T key;
        Node* left;
        Node* right;
        int height;
        Node(const T& k) : key(k), left(nullptr), right(nullptr), height(1) {}
    };

    Node* root;
    int count;

    int height(Node* n) const;
    int balanceFactor(Node* n) const;
    Node* rotateRight(Node* y);
    Node* rotateLeft(Node* x);
    Node* balance(Node* n);
    Node* insert(Node* n, const T& key);
    Node* remove(Node* n, const T& key);
    Node* minNode(Node* n) const;
    bool search(Node* n, const T& key) const;
    void destroy(Node* n);
};

#include "AVLTree.cpp"
