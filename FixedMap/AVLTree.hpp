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
#include <mutex>

// AVL Tree implementation
template<typename Key, typename Value, typename Compare = std::less<Key>>
class AVLTree : public BalancedTree<Key, Value, Compare> {
    using BaseTree = BalancedTree<Key, Value, Compare>;

protected:
    using Node = typename BaseTree::Node;

    Node *allocateNode() override {
        Node *node = BaseTree::allocateNode();
        node->SpecialProps.height = 1;// Leaf node height
        return node;
    }

    void deallocateNode(Node *node) override {
        BaseTree::deallocateNode(node);
        node->SpecialProps.height = 0;
    }

    int getHeight(Node *node) const { return node && node->in_use ? node->SpecialProps.height : 0; }

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
    int balanceFactor(Node *node) const { return node ? getHeight(node->left) - getHeight(node->right) : 0; }

    void updateHeight(Node *node) {
        if (node && node->in_use) {
            node->SpecialProps.height = std::max(getHeight(node->left), getHeight(node->right)) + 1;
        }
    }

    void rotateLeft(Node *node) override {
        Node *rightChild = node->right;
        BaseTree::rotateLeft(node);
        updateHeight(node);
        updateHeight(rightChild);
    }

    void rotateRight(Node *node) override {
        Node *leftChild = node->left;
        BaseTree::rotateRight(node);
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
    void balance(Node *node) {
        while (node) {
            updateHeight(node);
            int bf = balanceFactor(node);

            if (bf > 1) {                           // Left Sub-tree heavier
                if (balanceFactor(node->left) < 0) {//Left-right case - means that right child is heavier
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

public:
    AVLTree(Node *nodeBuffer, std::size_t capacity)
        : BaseTree(nodeBuffer, capacity) {}

    bool insert(const Key &key, const Value &value) override {
        if (BaseTree::m_size >= BaseTree::m_capacity) {
            return false;
        }
#if ENABLE_THREAD_SAFETY
        std::lock_guard<std::mutex> lock(BaseTree::m_mutex);
#endif

        Node *newNode = allocateNode();
        if (! BaseTree::insert(key, value, newNode)) {
            return false;
        }
        // Re-balance from parent up
        balance(newNode->parent);
        ++BaseTree::m_size;
        return true;
    }

    bool erase(const Key &key) override {
        Node *node = BaseTree::findNode(key);
        if (! node) {
            return false;
        }
#if ENABLE_THREAD_SAFETY
        std::lock_guard<std::mutex> lock(BaseTree::m_mutex);
#endif

        Node *parent = node->parent;

        if (! node->left || ! node->left->in_use) {
            BaseTree::transplant(node, node->right);
        } else if (! node->right || ! node->right->in_use) {
            BaseTree::transplant(node, node->left);
        } else {
            Node *successor = BaseTree::minimum(node->right);
            parent = successor->parent;
            Node *child = successor->right;

            if (successor->parent == node) {
                if (child) {
                    child->parent = successor;
                }
            } else {
                BaseTree::transplant(successor, successor->right);
                successor->right = node->right;
                successor->right->parent = successor;
            }
            BaseTree::transplant(node, successor);
            successor->left = node->left;
            successor->left->parent = successor;
        }

        deallocateNode(node);
        balance(parent);// Re-balance from parent up
        --BaseTree::m_size;
        return true;
    }
};
#endif//ESTL_AVLTREE_HPP
