//
// Created by SnirN on 3/24/2025.
//

#ifndef ESTL_AVLTREE_HPP
#define ESTL_AVLTREE_HPP
/** Simple Explanation of an AVL Tree
An AVL Tree is a special kind of binary search tree (BST) that keeps itself balanced. In a regular BST,
 keys are organized so that smaller keys go to the left and larger keys go to the right, but it can become unbalanced
 (like a long chain) if you add or remove items in a certain order. This makes operations like searching, inserting,
 or deleting slow. An AVL Tree fixes this by ensuring the tree stays balanced, so these operations always take
 about O(log n) time, where "n" is the number of items.

How It Stays Balanced
    Each node in the tree has a height, which is how many steps it takes to reach the deepest leaf below it.
    The balance factor of a node is the height of its left subtree minus the height of its right subtree.
    For an AVL Tree, this must always be -1, 0, or 1. If adding or removing a node makes the balance factor go beyond
    this range (like 2 or -2), the tree "rotates" nodes to fix the balance.

Rotations
    Rotations are like rearranging the tree to keep it short and wide instead of tall and skinny. There are four types:
        Left Rotation: Fixes a tree that’s too heavy on the right.
        Right Rotation: Fixes a tree that’s too heavy on the left.
        Left-Right Rotation: A mix of left then right, for tricky cases.
        Right-Left Rotation: A mix of right then left, for other tricky cases.

This balancing act ensures the tree never gets too lopsided, keeping operations fast.
 * **/

#include "IBalancedTree.hpp"
#include <stdexcept>

template<typename Key, typename Value>
struct AVLTreeNode : TreeNode<Key, Value> {
    AVLTreeNode *left = nullptr;
    AVLTreeNode *right = nullptr;
    AVLTreeNode *parent = nullptr;
    int height = 0;
};

// AVL Tree implementation
template<typename Key, typename Value, typename Compare = std::less<Key>>
class AVLTree : public BalancedTree<Key, Value, Compare> {
    using AVLNode = AVLTreeNode<Key, Value>;
    using GeneralNode = TreeNode<Key, Value>;

protected:
    AVLNode *m_nodes;
    AVLNode *m_root;
    AVLNode *m_freeNodes;
    Compare m_comparator;
    std::size_t m_size;
    std::size_t m_capacity;

    AVLNode *allocateNode() {
        if (! m_freeNodes) {
            throw std::out_of_range("No more free nodes available");
        }
        AVLNode *node = m_freeNodes;
        m_freeNodes = m_freeNodes->right;
        node->right = node->left = node->parent = nullptr;
        node->height = 1;// Leaf node height
        node->in_use = true;
        return node;
    }

    void deallocateNode(AVLNode *node) {
        node->in_use = false;
        node->height = 0;
        node->left = node->parent = nullptr;
        node->right = m_freeNodes;
        m_freeNodes = node;
    }

    int getHeight(AVLNode *node) const { return node && node->in_use ? node->height : 0; }

    /**
     * Calculates the balance factor of a given AVL tree node.
     *
     * The balance factor is defined as the difference in height between
     * the left and right subtrees of the node. A positive balance factor
     * indicates that the left subtree is taller, while a negative balance
     * factor indicates that the right subtree is taller.
     *
     * @param node A pointer to the AVL tree node for which the balance factor is calculated.
     * @return The balance factor of the node. If the node is null, returns 0.
     */
    int balanceFactor(AVLNode *node) const {
        return node ? getHeight(node->left) - getHeight(node->right) : 0;
    }

    void updateHeight(AVLNode *node) {
        if (node && node->in_use) {
            node->height = std::max(getHeight(node->left), getHeight(node->right)) + 1;
        }
    }

    void rotateLeft(AVLNode *node) {
        AVLNode *rightChild = node->right;
        node->right = rightChild->left;
        if (rightChild->left){
            rightChild->left->parent = node;
        }
        rightChild->parent = node->parent;

        if (! node->parent){
            m_root = rightChild;
        } else if (node == node->parent->left){
            node->parent->left = rightChild;
        } else {
            node->parent->right = rightChild;
        }

        rightChild->left = node;
        node->parent = rightChild;

        updateHeight(node);
        updateHeight(rightChild);
    }

    void rotateRight(AVLNode *node) {
        AVLNode *leftChild = node->left;
        node->left = leftChild->right;
        if (leftChild->right){
            leftChild->right->parent = node;
        }
        leftChild->parent = node->parent;

        if (! node->parent){
            m_root = leftChild;
        } else if (node == node->parent->right) {
            node->parent->right = leftChild;
        } else {
            node->parent->left = leftChild;
        }

        leftChild->right = node;
        node->parent = leftChild;

        updateHeight(node);
        updateHeight(leftChild);
    }

    /**
     * Balances the AVL tree starting from the given node and moving up to the root.
     *
     * This function updates the height of each node and calculates the balance factor.
     * If the balance factor is outside the range [-1, 1], it performs the necessary rotations
     * to restore the balance of the tree.
     *
     * @param node A pointer to the AVL tree node from which to start balancing.
     */
    void balance(AVLNode *node) {
        while (node) {
            updateHeight(node);
            int bf = balanceFactor(node);

            if (bf > 1) { // Left Sub-tree heavier
                if (balanceFactor(node->left) < 0) { //Left-right case - means that right child is heavier
                    rotateLeft(node->left);
                }
                rotateRight(node);
            } else if (bf < -1) {                    // Right heavy
                if (balanceFactor(node->right) > 0) {// Right-left case - means that left child is heavier
                    rotateRight(node->right);
                }
                rotateLeft(node);
            }
            node = node->parent;
        }
    }

    AVLNode *findNode(const Key &key) const {
        AVLNode *current = m_root;
        while (current && current->in_use) {
            if (m_comparator(key, current->key)){
                current = current->left;
            } else if (m_comparator(current->key, key)) {
                current = current->right;
            } else {
                return current;
            }
        }
        return nullptr;
    }

    AVLNode *minimum(AVLNode *node) const {
        if (! node || ! node->in_use){
            return nullptr;
        }
        auto current = static_cast<AVLNode*>(node);
        while (current->left && current->left->in_use){
            current = current->left;
        }
        return current;
    }

public:
    AVLTree(AVLNode *nodeBuffer, std::size_t capacity)
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

    bool insert(const Key &key, const Value &value) override {
        if (m_size >= m_capacity){
            return false;
        }

        AVLNode *newNode = allocateNode();
        newNode->key = key;
        newNode->value = value;
        //in case this is the first node
        if (! m_root) {
            m_root = newNode;
            ++m_size;
            return true;
        }
        //else insert as regular BST
        AVLNode *current = m_root;
        AVLNode *parent = nullptr;
        while (current) {
            parent = current;
            if (m_comparator(key, current->key)){ // is key < current->key
                current = current->left;
            } else if (m_comparator(current->key, key)) {// is current>key < key
                current = current->right;
            } else { //key == current->key
                deallocateNode(newNode);
                return false;// Duplicate key
            }
        }

        newNode->parent = parent;
        if (m_comparator(key, parent->key)){
            parent->left = newNode;
        } else {
            parent->right = newNode;
        }
        // Re-balance from parent up
        balance(newNode->parent);
        ++m_size;
        return true;
    }

    bool erase(const Key &key) override {
        AVLNode *node = findNode(key);
        if (! node){
            return false;
        }

        AVLNode *parent = node->parent;

        if (! node->left || ! node->left->in_use) {
            transplant(node, node->right);
        } else if (! node->right || ! node->right->in_use) {
            transplant(node, node->left);
        } else {
            AVLNode *successor = minimum(node->right);
            parent = successor->parent;
            AVLNode* child = successor->right;

            if (successor->parent == node) {
                if (child) {
                    child->parent = successor;
                }
            } else {
                transplant(successor, successor->right);
                successor->right = node->right;
                successor->right->parent = successor;
            }
            transplant(node, successor);
            successor->left = node->left;
            successor->left->parent = successor;
        }

        deallocateNode(node);
        balance(parent);// Rebalance from parent up
        --m_size;
        return true;
    }

    void transplant(AVLNode *u, AVLNode *v) {
        if (! u->parent) {
            m_root = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }

        if (v){
            v->parent = u->parent;
        }
    }

    Value *find(const Key &key) override {
        AVLNode *node = findNode(key);
        return node ? &node->value : nullptr;
    }

    void clear() override {
        for (std::size_t i = 0; i < m_capacity; ++i) {
            if (m_nodes[i].in_use)
                deallocateNode(&m_nodes[i]);
        }
        m_root = nullptr;
        m_size = 0;
    }

    std::size_t size() const override { return m_size; }
    bool empty() const override { return m_size == 0; }

    AVLNode *minimum() override { return minimum(m_root); }

    AVLNode *next(GeneralNode *node) override {
        if (! node || ! node->in_use){
            return nullptr;
        }
        auto current = static_cast<AVLNode*>(node);
        if (current->right && current->right->in_use)
            return minimum(current->right);
        AVLNode *parent = current->parent;
        while (parent && current == parent->right) {
            current = parent;
            parent = parent->parent;
        }
        return parent;
    }

    AVLNode *prev(GeneralNode *node) override {
        if (! node || ! node->in_use) {
            AVLNode *current = m_root;
            while (current && current->right && current->right->in_use)
                current = current->right;
            return current;
        }
        auto currentN = static_cast<AVLNode*>(node);
        if (currentN->left && currentN->left->in_use) {
            AVLNode *current = currentN->left;
            while (current->right && current->right->in_use)
                current = current->right;
            return current;
        }
        AVLNode *parent = currentN->parent;
        while (parent && currentN == parent->left) {
            currentN = parent;
            parent = parent->parent;
        }
        return parent;
    }
};
#endif//ESTL_AVLTREE_HPP
