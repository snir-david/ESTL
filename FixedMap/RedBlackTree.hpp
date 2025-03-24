//
// Created by SnirN on 3/21/2025.
//

#ifndef ESTL_REDBLACKTREE_HPP
#define ESTL_REDBLACKTREE_HPP
#pragma once

/** Simple Explanation of Red-Black Tree (RB Tree)
A Red-Black Tree is a type of self-balancing binary search tree (BST) that ensures efficient operations (like insertion,
deletion, and search) with a time complexity of O(log n). It maintains balance by enforcing specific rules, avoiding
the worst-case scenarios of an unbalanced BST (where it could degrade to O(n)).
Key Properties
    Each node in an RB Tree has:
        1. A key and value (like any BST).
        2. A color: Either red or black.
        3. Pointers to left child, right child, and parent.
    The tree stays balanced by following these rules:
        1. Root is Black: The root node is always black.
        2. Red Rule: A red node cannot have a red child (no two reds in a row).
        3. Black Depth: Every path from the root to a leaf (null node) has the same number of black nodes.
        4. Leaf Nodes: All null (empty) nodes are considered black (though in practice, we often don’t store them
        explicitly).
How It Balances
    Insertion: When adding a node, it starts as red. If this violates the "no two reds" rule, the tree rotates nodes (left or right) and recolors them to restore balance.
    Deletion: When removing a node, the tree adjusts colors and rotates to maintain the black depth rule.
    Rotations: Left or right shifts in the tree structure to fix imbalances (e.g., too many nodes on one side).
 * */

#include "IBalancedTree.hpp"
#include <array>
#include <functional>
#include <stdexcept>

template<typename Key, typename Value>
struct RBTreeNode : TreeNode<Key, Value> {
    RBTreeNode *left = nullptr;
    RBTreeNode *right = nullptr;
    RBTreeNode *parent = nullptr;
    bool red = false;// For red-black tree
};

// Red-Black Tree implementation
template<typename Key, typename Value, typename Compare = std::less<Key>>
class RBTree : public BalancedTree<Key, Value, Compare> {
    using RBNode = RBTreeNode<Key, Value>;
    using GeneralNode = TreeNode<Key, Value>;

protected:
    RBNode *m_nodes;
    RBNode *m_root;
    RBNode *m_freeNodes;
    Compare m_comparator;
    std::size_t m_size;
    std::size_t m_capacity;

    RBNode *allocateNode() {
        if (! m_freeNodes) {
            throw std::out_of_range("No more free nodes available");
        }
        RBNode *node = m_freeNodes;
        m_freeNodes = m_freeNodes->right;
        node->right = node->left = node->parent = nullptr;
        node->red = node->in_use = true;
        return node;
    }

    void deallocateNode(RBNode *node) {
        node->in_use = node->red = false;
        node->left = node->parent = nullptr;
        node->right = m_freeNodes;
        m_freeNodes = node;
    }

    /** @brief Fixes the Red-Black Tree after an insertion to maintain balance.
     *
     * This function ensures that the Red-Black Tree properties are maintained after a new node is inserted.
     * It uses a helper lambda function to handle the balancing cases and performs rotations and recoloring as needed.
     *
     * @param node The newly inserted node that may cause a violation of the Red-Black Tree properties.
     */
    void balanceAfterInsertion(RBNode *node) {
        /** @brief Helper lambda function to balance the tree.
     *
     * This function handles the balancing cases when the uncle node is red or black.
     * It performs rotations and recoloring to restore the Red-Black Tree properties.
     *
     * @param node The current node being checked.
     * @param uncle The uncle node of the current node.
     * @param left A boolean indicating if the current node is a left child.
     */
        auto balanceHelper = [this](RBNode *node, RBNode *uncle, bool isParentLeft) {
            if (uncle && uncle->red) {                 // Case 1: Red uncle and parent (2 reds in a row)
                node->parent->red = uncle->red = false;// recolor parent & uncle as black
                node->parent->parent->red = true;      // recolor grandparent as red
                node = node->parent->parent;// node == grandparent, go back and make sure now that the changes left
                                            // the tree balanced
            } else {                        // Case 2 or 3: Black uncle
                /**    GP              GP
                 *    /  \            /  \
                 *       P     OR    P
                 *      / \         / \
                 *     Node           Node
                 * **/
                if (isParentLeft ? (node == node->parent->right) : (node == node->parent->left)) {// Case 2: Triangle -
                    // current node, parent and grandparent form a  triangle
                    node = node->parent;
                    //rotate the parent to fix the triangle
                    isParentLeft ? rotateLeft(node) : rotateRight(node);
                }
                /**    GP              GP
                 *    /  \            /  \
                 *       P     OR    P
                 *      / \         / \
                 *        Node    Node
                 * **/
                node->parent->red = false;// Case 3: Line
                node->parent->parent->red = true;
                //rotate the grandparent to fix the line
                isParentLeft ? rotateRight(node->parent->parent) : rotateLeft(node->parent->parent);
            }
        };

        // Loop to fix the Red-Black Tree properties if the parent node is red
        while (node != m_root && node->parent->red) {// Rule 2 violation: red parent
            if (node->parent == node->parent->parent->left) {
                balanceHelper(node, node->parent->parent->right, true);
            } else {// Symmetric case (right side)
                balanceHelper(node, node->parent->parent->left, false);
            }
        }
        m_root->red = false;// Ensure the root is always black
    }

    /** @brief Performs a left rotation on the given node.
     *
     * This function rotates the given node to the left, adjusting the pointers
     * of the node, its right child, and their parents to maintain the Red-Black Tree properties.
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
    void rotateLeft(RBNode *node) {
        RBNode *rightChild = node->right;
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
     * of the node, its left child, and their parents to maintain the Red-Black Tree properties.
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
    void rotateRight(RBNode *node) {
        RBNode *leftChild = node->left;
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

    RBNode *findNode(const Key &key) const {
        RBNode *current = m_root;
        while (current && current->in_use) {
            if (m_comparator(key, current->key)) {// if key < current->key
                current = current->left;
            } else if (m_comparator(current->key, key)) {// else if current->key < key
                current = current->right;
            } else {// else key == current->key
                return current;
            }
        }
        return nullptr;
    }

    /**
     * @brief Balances the Red-Black Tree after a node deletion.
     *
     * This function ensures that the Red-Black Tree properties are maintained after a node is deleted.
     * It uses a helper lambda function to handle the balancing cases and performs rotations and recoloring as needed.
     *
     * @param node The node that may cause a violation of the Red-Black Tree properties after deletion.
     */
    void balanceAfterDeletion(RBNode *node) {
        /** @brief Helper lambda function to balance the tree after deletion.
         *
         * This function handles the balancing cases when the sibling node is red or black.
         * It performs rotations and recoloring to restore the Red-Black Tree properties.
         *
         * @param node The current node being checked.
         * @param sibling The sibling node of the current node. Sibling is the other child of the parent, for example -
         *            Parent
         *           /     \
         *          Node   Sibling
         * @param isSiblingLeft A boolean indicating if the sibling node is a left child.
         */
        auto balanceHelper = [this](RBNode *node, RBNode *sibling, bool isSiblingLeft) {
            if (sibling && sibling->red) {// Case 1: Red sibling
                sibling->red = false;
                node->parent->red = true;
                isSiblingLeft ? rotateRight(node->parent) : rotateLeft(node->parent);
                sibling = isSiblingLeft ? node->parent->left : node->parent->right;
            }
            if ((! sibling->left || ! sibling->left->red) &&
                (! sibling->right || ! sibling->right->red)) {//Case 2: Black sibling, both children are black
                sibling->red = true;
                node = node->parent;
            } else {
                if (isSiblingLeft ? (! sibling->left || ! sibling->left->red)
                                  : (! sibling->right ||
                                     ! sibling->right->red)) {//Case 3: Black sibling, sibling's child is black
                    isSiblingLeft ? sibling->right->red : sibling->left->red = false;
                    sibling->red = true;
                    isSiblingLeft ? rotateLeft(sibling) : rotateRight(sibling);
                    sibling = isSiblingLeft ? node->parent->left : node->parent->right;
                }
                //Case 4: Black sibling, sibling's child is red
                sibling->red = node->parent->red;
                node->parent->red = false;
                isSiblingLeft ? sibling->left->red : sibling->right->red = false;
                isSiblingLeft ? rotateRight(node->parent) : rotateLeft(node->parent);
                node = m_root;
            }
        };
        //loop to fix the tree after deletion
        while (node != m_root && (! node || ! node->red)) {//only if black node deleted
            if (node == node->parent->left) {
                balanceHelper(node, node->parent->right, false);
            } else {
                balanceHelper(node, node->parent->left, true);
            }
        }

        if (node) {
            node->red = false;
        }
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
    void transplant(RBNode *u, RBNode *v) {
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

public:
    RBTree(RBNode *nodeBuffer, std::size_t capacity)
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
        if (m_size >= m_capacity) {
            return false;
        }

        RBNode *newNode = allocateNode();
        newNode->key = key;
        newNode->value = value;
        //in case this is the first node
        if (! m_root) {
            m_root = newNode;
            m_root->red = false;
            ++m_size;
            return true;
        }
        //else insert as regular BST
        RBNode *current = m_root;
        RBNode *parent = nullptr;
        while (current) {
            parent = current;
            if (m_comparator(key, current->key))// is key < current->key
                current = current->left;
            else if (m_comparator(current->key, key))// is current>key < key
                current = current->right;
            else {//key == current->key
                deallocateNode(newNode);
                return false; // Duplicate key
            }
        }

        newNode->parent = parent;
        if (m_comparator(key, parent->key)) {
            parent->left = newNode;
        } else {
            parent->right = newNode;
        }
        // after insertion, balance the tree
        balanceAfterInsertion(newNode);
        ++m_size;
        return true;
    }

    bool erase(const Key &key) override {
        RBNode *node = findNode(key);
        if (! node) {// node doesn't exists in tree
            return false;
        }

        RBNode *child;
        bool originalColor = node->red;

        if (! node->left) {// Case left child is null - call to transplant for the right child (and subtree)
            child = node->right;
            transplant(node, node->right);
        } else if (! node->right) {// Case right child is null  - call to transplant for the left child (and subtree)
            child = node->left;
            transplant(node, node->left);
        } else {// Case none of the children are null
            //find minimum in right subtree
            RBNode *successor = minimum(node->right);
            originalColor = successor->red;
            child = successor->right;
            if (successor->parent == node) {//successor is the right child of node
                if (child) {
                    child->parent = successor;
                }
            } else {//successor is not the right child of node, need to adjust the tree
                transplant(successor, successor->right);
                successor->right = node->right;
                successor->right->parent = successor;
            }
            //replace node with successor
            transplant(node, successor);
            successor->left = node->left;
            successor->left->parent = successor;
            successor->red = node->red;
        }

        deallocateNode(node);
        if (! originalColor && child) {//only if black node deleted
            balanceAfterDeletion(child);
        }
        --m_size;
        return true;
    }

    Value *find(const Key &key) override {
        RBNode *node = findNode(key);
        return node ? &node->value : nullptr;
    }

    void clear() override {
        for (std::size_t i = 0; i < m_capacity; ++i) {
            if (m_nodes[i].in_use) {
                deallocateNode(&m_nodes[i]);
            }
        }
        m_root = nullptr;
        m_size = 0;
    }

    std::size_t size() const override { return m_size; }
    bool empty() const override { return m_size == 0; }

    /**
     * @brief Finds the minimum node in the subtree rooted at the given node.
     *
     * This function traverses the left children of the given node to find the node
     * with the smallest key in the subtree. It returns nullptr if the node is null
     * or not in use.
     *
     * @param node The root of the subtree to search for the minimum node.
     * @return The node with the smallest key in the subtree, or nullptr if the node is null or not in use.
     */
    RBNode *minimum(GeneralNode *node) const {
        if (! node || ! node->in_use) {
            return nullptr;
        }
        auto current = static_cast<RBNode *>(node);
        while (current->left && current->left->in_use) {
            current = current->left;
        }
        return current;
    }

    RBNode *minimum() override { return minimum(m_root); }

    /**
     * @brief Finds the successor of the given node in the Red-Black Tree.
     *
     * This function returns the next node in the in-order traversal of the tree.
     * If the given node is null or not in use, it returns nullptr.
     * If the right child of the given node is not null, it returns the minimum node in the right subtree.
     * Otherwise, it traverses up the tree to find the first ancestor that is a left child of its parent.
     *
     * @param node The node for which the successor is to be found.
     * @return The successor node, or nullptr if no successor exists.
     */
    RBNode *next(GeneralNode *node) override {
        if (! node || ! node->in_use) {
            return nullptr;
        }

        auto current = static_cast<RBNode *>(node);
        if (current->right && current->right->in_use) {
            return minimum(current->right);
        }

        RBNode *parent = current->parent;
        while (parent && current == parent->right) {
            current = parent;
            parent = parent->parent;
        }
        return parent;
    }

    /**
     * @brief Finds the predecessor of the given node in the Red-Black Tree.
     *
     * This function returns the previous node in the in-order traversal of the tree.
     * If the given node is null or not in use, it returns the maximum node in the tree.
     * If the left child of the given node is not null, it returns the maximum node in the left subtree.
     * Otherwise, it traverses up the tree to find the first ancestor that is a right child of its parent.
     *
     * @param node The node for which the predecessor is to be found.
     * @return The predecessor node, or nullptr if no predecessor exists.
     */
    RBNode *prev(GeneralNode *node) override {
        if (! node || ! node->in_use) {
            RBNode *current = m_root;
            while (current && current->right && current->right->in_use) {
                current = current->right;
            }
            return current;
        }
        auto currentN = static_cast<RBNode *>(node);
        if (currentN->left && currentN->left->in_use) {
            RBNode *current = currentN->left;
            while (current->right && current->right->in_use) {
                current = current->right;
            }
            return current;
        }

        RBNode *parent = currentN->parent;
        while (parent && currentN == parent->left) {
            currentN = parent;
            parent = parent->parent;
        }
        return parent;
    }
};


#endif//ESTL_REDBLACKTREE_HPP
