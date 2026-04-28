#pragma once

template <typename T>
class RedBlackTree {
public:
    RedBlackTree();
    ~RedBlackTree();

    void insert(const T& key);
    void remove(const T& key);
    bool search(const T& key) const;
    int size() const;

private:
    enum Color { RED, BLACK };

    struct Node {
        T key;
        Color color;
        Node* left;
        Node* right;
        Node* parent;
        Node(const T& k)
            : key(k), color(RED), left(nullptr), right(nullptr), parent(nullptr) {}
    };

    Node* root;
    Node* nil;
    int count;

    void leftRotate(Node* x);
    void rightRotate(Node* x);
    void insertFixup(Node* z);
    void transplant(Node* u, Node* v);
    Node* minimum(Node* x) const;
    void removeFixup(Node* x);
    bool search(Node* n, const T& key) const;
    void destroy(Node* n);
};

#include "RedBlackTree.cpp"
