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

// Red-Black Tree implementation
template<typename Key, typename Value, typename Compare = std::less<Key>>
class RBTree : public BalancedTree<Key, Value, Compare> {
    using BaseTree = BalancedTree<Key, Value, Compare>;

protected:
    using Node = typename BaseTree::Node;

    Node *allocateNode() override {
        Node *node = BaseTree::allocateNode();
        node->SpecialProps.red = true;
        return node;
    }

    void deallocateNode(Node *node) override {
        BaseTree::deallocateNode(node);
        node->SpecialProps.red = false;
    }

    void rotateLeft(Node *node, Node* &parent) {
      parent = node->parent;
      BaseTree::rotateLeft(parent);
      node = parent;
      parent = node->parent;
    }

    void rotateRight(Node *node, Node* &parent) {
      parent = node->parent;
      BaseTree::rotateRight(parent);
      node = parent;
      parent = node->parent;
    }

    /** @brief Fixes the Red-Black Tree after an insertion to maintain balance.
     *
     * This function ensures that the Red-Black Tree properties are maintained after a new node is inserted.
     * It uses a helper lambda function to handle the balancing cases and performs rotations and recoloring as needed.
     *
     * @param node The newly inserted node that may cause a violation of the Red-Black Tree properties.
     */
    void balanceAfterInsertion(Node *node) {
        /** @brief Helper lambda function to balance the tree.
     *
     * This function handles the balancing cases when the uncle node is red or black.
     * It performs rotations and recoloring to restore the Red-Black Tree properties.
     *
     * @param node The current node being checked.
     * @param uncle The uncle node of the current node.
     * @param left A boolean indicating if the current node is a left child.
     */
        auto balanceHelper = [this, &node](Node *uncle, bool
                                                                   isParentLeft) {
          // Exit if no parent or grandparent
            if (!node->parent || !node->parent->parent) {
              return;
            }

            auto parent = node->parent;
            auto grandparent = node->parent->parent;
            if (uncle && uncle->SpecialProps.red) {// Case 1: Red uncle and parent (2 reds in a row)
                parent->SpecialProps.red = uncle->SpecialProps.red = false;// recolor parent & uncle as black
                grandparent->SpecialProps.red = true;                      // recolor grandparent as red
                node = grandparent;// node == grandparent, go back and make sure now that the changes left
                                   // the tree balanced
            } else {               // Case 2 or 3: Black uncle
                /**    GP              GP
                 *    /  \            /  \
                 *       P     OR    P
                 *      / \         / \
                 *     Node           Node
                 * **/
                if (isParentLeft ? (node == parent->right) : (node == parent->left)) {// Case 2: Triangle -
                    // current node, parent and grandparent form a  triangle
                    //rotate the parent to fix the triangle
                  isParentLeft ? rotateLeft(node, parent)
                               : rotateRight(node, parent);
                }
                /**    GP              GP
                 *    /  \            /  \
                 *       P     OR    P
                 *      / \         / \
                 *        Node    Node
                 * **/
                parent->SpecialProps.red = false;// Case 3: Line
                grandparent->SpecialProps.red = true;
                //rotate the grandparent to fix the line
                isParentLeft ? BaseTree::rotateRight(grandparent) : BaseTree::rotateLeft(grandparent);
            }
        };

        // Loop to fix the Red-Black Tree properties if the parent node is red
        while (node != BaseTree::m_root && node->parent && node->parent->SpecialProps.red) {// Rule 2 violation: red parent
          if (node->parent->parent &&
              node->parent == node->parent->parent->left) {
            balanceHelper(node->parent->parent->right, true);
            } else {// Symmetric case (right side)
                balanceHelper(node->parent->parent->left, false);
            }
        }
        BaseTree::m_root->SpecialProps.red = false;// Ensure the root is always black
    }

    /**
     * @brief Balances the Red-Black Tree after a node deletion.
     *
     * This function ensures that the Red-Black Tree properties are maintained after a node is deleted.
     * It uses a helper lambda function to handle the balancing cases and performs rotations and recoloring as needed.
     *
     * @param node The node that may cause a violation of the Red-Black Tree properties after deletion.
     */
    void balanceAfterDeletion(Node *node) {
      Node *sibling;
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
      auto balanceHelper = [this, &node, &sibling](bool isSiblingLeft) {
        if (!node || !node->parent) return;

        // Case 1: Red sibling
        if (sibling && sibling->SpecialProps.red) {
          sibling->SpecialProps.red = false;
          node->parent->SpecialProps.red = true;
          isSiblingLeft ? BaseTree::rotateRight(node->parent) : BaseTree::rotateLeft(node->parent);
          sibling = isSiblingLeft ? node->parent->left : node->parent->right;
        }

        // If sibling is null, move up
        if (!sibling) {
          node = node->parent;
          return;
        }

        Node *leftNephew = sibling->left;
        Node *rightNephew = sibling->right;

        // Case 2: Black sibling, both children black
        if ((!leftNephew || !leftNephew->SpecialProps.red) &&
            (!rightNephew || !rightNephew->SpecialProps.red)) {
          sibling->SpecialProps.red = true;
          node = node->parent;
          return;
        }

        // Case 3/4: Black sibling, at least one red child
        if (isSiblingLeft ? !leftNephew || !leftNephew->SpecialProps.red :
                          !rightNephew || !rightNephew->SpecialProps.red) {
          // Case 3: Left nephew black or null
            if (isSiblingLeft ? rightNephew : leftNephew){
              isSiblingLeft? rightNephew->SpecialProps.red :
                            leftNephew->SpecialProps.red = false;
            }

            sibling->SpecialProps.red = true;
            isSiblingLeft ? BaseTree::rotateLeft(sibling) :
                          BaseTree::rotateRight(sibling);
            sibling = isSiblingLeft ? node->parent->left : node->parent->right;
            isSiblingLeft ? leftNephew = sibling->left : rightNephew = sibling->right;
          }
          // Case 4: Left nephew red
          sibling->SpecialProps.red = node->parent->SpecialProps.red;
          node->parent->SpecialProps.red = false;
          if (isSiblingLeft ? leftNephew : rightNephew) {
            isSiblingLeft ? leftNephew->SpecialProps.red :
                          rightNephew->SpecialProps.red =  false;
          }
          isSiblingLeft ? BaseTree::rotateRight(node->parent) :
                        BaseTree::rotateLeft(node->parent);
          node = BaseTree::m_root;
      };

      while (node && node != BaseTree::m_root && !node->SpecialProps.red) {
        if (!node->parent) break;
        if (node == node->parent->left) {
          sibling = node->parent->right;
          balanceHelper(false);
        } else {
          sibling = node->parent->left;
          balanceHelper(true);
        }
      }

      if (node) node->SpecialProps.red = false;
    }

public:
    RBTree(Node *nodeBuffer, std::size_t capacity)
        : BaseTree(nodeBuffer, capacity) {}

    bool insert(const Key &key, const Value &value) override {
        if (BaseTree::m_size >= BaseTree::m_capacity) {
            return false;
        }

        Node *newNode = allocateNode();
        if (! BaseTree::insert(key, value, newNode)) {
            return false;
        }
        // after insertion, balance the tree
        balanceAfterInsertion(newNode);
        ++BaseTree::m_size;
        return true;
    }

    bool erase(const Key &key) override {
        Node *node = BaseTree::findNode(key);
        if (! node) {// node doesn't exists in tree
            return false;
        }

        Node *child;
        bool originalColor = node->SpecialProps.red;

        if (! node->left) {// Case left child is null - call to transplant for the right child (and subtree)
            child = node->right;
            BaseTree::transplant(node, node->right);
        } else if (! node->right) {// Case right child is null  - call to transplant for the left child (and subtree)
            child = node->left;
            BaseTree::transplant(node, node->left);
        } else {// Case none of the children are null
            //find minimum in right subtree
            Node *successor = BaseTree::minimum(node->right);
            originalColor = successor->SpecialProps.red;
            child = successor->right;
            if (successor->parent == node) {//successor is the right child of node
                if (child) {
                    child->parent = successor;
                }
            } else {//successor is not the right child of node, need to adjust the tree
                BaseTree::transplant(successor, successor->right);
                successor->right = node->right;
                successor->right->parent = successor;
            }
            //replace node with successor
            BaseTree::transplant(node, successor);
            successor->left = node->left;
            successor->left->parent = successor;
            successor->SpecialProps.red = node->SpecialProps.red;
        }

        deallocateNode(node);
        if (! originalColor && child) {//only if black node deleted
            balanceAfterDeletion(child);
        }
        --BaseTree::m_size;
        return true;
    }
};


#endif//ESTL_REDBLACKTREE_HPP
