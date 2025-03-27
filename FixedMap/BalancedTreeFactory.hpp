//
// Created by SnirN on 3/23/2025.
//

#ifndef ESTL_BALANCEDTREEFACTORY_HPP
#define ESTL_BALANCEDTREEFACTORY_HPP

#include "AVLTree.hpp"
#include "RedBlackTree.hpp"
#include <memory>

enum TreeType { RedBlack, AVL };


template<typename Key, typename Value, typename Compare = std::less<Key>>
class BalancedTreeFactory {
public:
    static std::unique_ptr<BalancedTree<Key, Value, Compare>> createTree(TreeType type,TreeNode<Key, Value> *buffer , std::size_t capacity) {
        switch (type) {
            case TreeType::RedBlack:
                return std::move(
                        std::make_unique<RBTree<Key, Value, Compare>>(buffer, capacity));
            case TreeType::AVL:
                return std::move(
                        std::make_unique<AVLTree<Key, Value, Compare>>(buffer, capacity));
            default:
                throw std::invalid_argument("Unknown tree type");
        }
    }
};
#endif//ESTL_BALANCEDTREEFACTORY_HPP
