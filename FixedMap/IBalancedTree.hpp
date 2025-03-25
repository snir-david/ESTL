//
// Created by SnirN on 3/21/2025.
//

#ifndef ESTL_IBALANCEDTREE_HPP
#define ESTL_IBALANCEDTREE_HPP
#pragma once

#include <functional>
#include <stdexcept>
#include <utility>

template<typename Key, typename Value>
struct TreeNode {
    Key key;
    Value value;
    bool in_use = false;
    TreeNode *left = nullptr;
    TreeNode *right = nullptr;
    TreeNode *parent = nullptr;

    union {
        bool red;  // For red-black tree
        int height;// For AVL tree
    } SpecialProps;
};

// Abstract Balanced Tree interface
template<typename Key, typename Value, typename Compare = std::less<Key>>
class BalancedTree {
protected:
    using Node = TreeNode<Key, Value>;

    Node *m_root;
    Node *m_nodes;
    Node *m_freeNodes;
    std::size_t m_size;
    std::size_t m_capacity;
    Compare m_comparator;

    bool insert(const Key &key, const Value &value, Node *newNode) {
        newNode->key = key;
        newNode->value = value;
        //in case this is the first node
        if (! m_root) {
            m_root = newNode;
            return true;
        }
        //else insert as regular BST
        Node *current = m_root;
        Node *parent = nullptr;
        while (current) {
            parent = current;
            if (m_comparator(key, current->key)) {// is key < current->key
                current = current->left;
            } else if (m_comparator(current->key, key)) {// is current>key < key
                current = current->right;
            } else {//key == current->key
                deallocateNode(newNode);
                return false;// Duplicate key
            }
        }

        newNode->parent = parent;
        if (m_comparator(key, parent->key)) {
            parent->left = newNode;
        } else {
            parent->right = newNode;
        }
        return true;
    };

public:
    BalancedTree(Node *nodeBuffer, std::size_t capacity)
        : m_nodes(nodeBuffer)
        , m_root(nullptr)
        , m_freeNodes(nullptr)
        , m_size(0)
        , m_capacity(capacity) {
        for (std::size_t i = 0; i < m_capacity - 1; ++i) {
            m_nodes[i].right = &m_nodes[i + 1];
        }
        m_nodes[m_capacity - 1].right = nullptr;
        m_freeNodes = &m_nodes[0];
    }
    virtual ~BalancedTree(){
        delete[] m_nodes;
    };

    virtual bool insert(const Key &key, const Value &value) = 0;
    virtual bool erase(const Key &key) = 0;

    virtual Node *allocateNode() {
        if (! m_freeNodes) {
            throw std::out_of_range("No more free nodes available");
        }
        Node *node = m_freeNodes;
        m_freeNodes = m_freeNodes->right;
        node->right = node->left = node->parent = nullptr;
        node->in_use = true;
        return node;
    }

    virtual void deallocateNode(Node *node) {
        node->in_use = false;
        node->left = node->parent = nullptr;
        node->right = m_freeNodes;
        m_freeNodes = node;
    }

    virtual Value *find(const Key &key) {
        Node *node = findNode(key);
        return node ? &node->value : nullptr;
    }

    virtual void clear() {
        for (std::size_t i = 0; i < m_capacity; ++i) {
            if (m_nodes[i].in_use)
                deallocateNode(&m_nodes[i]);
        }
        m_root = nullptr;
        m_size = 0;
    }

    std::size_t size() const { return m_size; }
    bool empty() const { return m_size == 0; }

    /** @brief Performs a left rotation on the given node.
     *
     * This function rotates the given node to the left, adjusting the pointers
     * of the node, its right child, and their parents to maintain the Balanced Tree properties.
    *      Grandparent    Grandparent
    *         /      \         /     \
    *      Parent          Parent
    *      /    \          /   \
    *          Node            R
    *         /   \    -->    /  \
    *         L    R        Node RR
    *        / \  / \      /\
    *       LL RL RL RR   L RL
     * @param node The node to be rotated to the left.
     */
    virtual void rotateLeft(Node *node) {
        Node *rightChild = node->right;
        node->right = rightChild->left;

        if (rightChild->left) {
            rightChild->left->parent = node;
        }
        rightChild->parent = node->parent;
        //case it's the root, need to change to new root
        if (! node->parent) {
            m_root = rightChild;
        } else if (node == node->parent->left) {
            node->parent->left = rightChild;
        } else {
            node->parent->right = rightChild;
        }

        rightChild->left = node;
        node->parent = rightChild;
    }

    /** @brief Performs a right rotation on the given node.
     *
     * This function rotates the given node to the right, adjusting the pointers
     * of the node, its left child, and their parents to maintain the Balanced Tree properties.
     *      Grandparent    Grandparent
     *        /      \         /     \
     *      Parent          Parent
    *        /  \            /  \
    *          Node              L
    *         /   \    -->    /  \
    *         L    R        LL   Node
    *        / \  / \             /\
    *       LL RL RL RR          LR R
     * @param node The node to be rotated to the right.
     */
    virtual void rotateRight(Node *node) {
        Node *leftChild = node->left;
        node->left = leftChild->right;

        if (leftChild->right) {
            leftChild->right->parent = node;
        }
        leftChild->parent = node->parent;

        if (! node->parent) {
            m_root = leftChild;
        } else if (node == node->parent->right) {
            node->parent->right = leftChild;
        } else {
            node->parent->left = leftChild;
        }

        leftChild->right = node;
        node->parent = leftChild;
    }

    /**
     * @brief Replaces one subtree as a child of its parent with another subtree.
     *
     * This function replaces the subtree rooted at node `u` with the subtree rooted at node `v`.
     * It adjusts the parent pointers to ensure the tree remains connected correctly.
     *
     * @param u The node to be replaced.
     * @param v The node to replace `u`.
     *
     * @example
     * Before transplant:        Transplant node 20 with node 30:
     *      P                                  P
     *    /  \                               /  \
     *   5   u                             5     v
     *        \
     *        v
     */
    void transplant(Node *u, Node *v) {
        if (! u->parent) {
            m_root = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }

        if (v) {
            v->parent = u->parent;
        }
    }

    virtual Node *findNode(const Key &key) const {
        Node *current = m_root;
        while (current && current->in_use) {
            if (m_comparator(key, current->key)) {
                current = current->left;
            } else if (m_comparator(current->key, key)) {
                current = current->right;
            } else {
                return current;
            }
        }
        return nullptr;
    }

    virtual Node *minimum(Node *node) const {
        if (! node || ! node->in_use) {
            return nullptr;
        }
        while (node->left && node->left->in_use) {
            node = node->left;
        }
        return node;
    }

    // For iteration
    Node *minimum() { return minimum(m_root); }

    /**
     * @brief Finds the successor of the given node in the Balanced Tree.
     *
     * This function returns the next node in the in-order traversal of the tree.
     * If the given node is null or not in use, it returns nullptr.
     * If the right child of the given node is not null, it returns the minimum node in the right subtree.
     * Otherwise, it traverses up the tree to find the first ancestor that is a left child of its parent.
     *
     * @param node The node for which the successor is to be found.
     * @return The successor node, or nullptr if no successor exists.
     */
    virtual Node *next(Node *node) {
        if (! node || ! node->in_use) {
            return nullptr;
        }

        if (node->right && node->right->in_use) {
            return minimum(node->right);
        }

        Node *parent = node->parent;
        while (parent && node == parent->right) {
            node = parent;
            parent = parent->parent;
        }
        return parent;
    }

    /**
     * @brief Finds the predecessor of the given node in the Balanced Tree.
     *
     * This function returns the previous node in the in-order traversal of the tree.
     * If the given node is null or not in use, it returns the maximum node in the tree.
     * If the left child of the given node is not null, it returns the maximum node in the left subtree.
     * Otherwise, it traverses up the tree to find the first ancestor that is a right child of its parent.
     *
     * @param node The node for which the predecessor is to be found.
     * @return The predecessor node, or nullptr if no predecessor exists.
     */
    virtual Node *prev(Node *node) {
        if (! node || ! node->in_use) {
            Node *current = m_root;
            while (current && current->right && current->right->in_use) {
                current = current->right;
            }
            return current;
        }
        if (node->left && node->left->in_use) {
            Node *current = node->left;
            while (current->right && current->right->in_use) {
                current = current->right;
            }
            return current;
        }

        Node *parent = node->parent;
        while (parent && node == parent->left) {
            node = parent;
            parent = parent->parent;
        }
        return parent;
    }
};
#endif//ESTL_IBALANCEDTREE_HPP
