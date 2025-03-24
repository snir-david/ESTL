//
// Created by SnirN on 3/23/2025.
//

#ifndef ESTL_BALANCEDTREEFACTORY_HPP
#define ESTL_BALANCEDTREEFACTORY_HPP

#include "RedBlackTree.hpp"
#include <memory>

enum TreeType { RedBlackTree, AVLTree };


template<typename Key, typename Value, typename Compare = std::less<Key>>
class BalancedTreeFactory {
public:
    static std::unique_ptr<BalancedTree<Key, Value, Compare>> createTree(TreeType type, std::size_t capacity) {
        switch (type) {
            case RedBlackTree:
                return std::move(
                        std::make_unique<RBTree<Key, Value, Compare>>(new RBTreeNode<Key, Value>[capacity], capacity));
            default:
                throw std::invalid_argument("Unknown tree type");
        }
    }
};
#endif//ESTL_BALANCEDTREEFACTORY_HPP
