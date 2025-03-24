//
// Created by SnirN on 3/21/2025.
//

#ifndef ESTL_IBALANCEDTREE_HPP
#define ESTL_IBALANCEDTREE_HPP
#pragma once

#include <functional>
#include <utility>

template<typename Key, typename Value>
struct TreeNode {
    Key key;
    Value value;
    bool in_use = false;
    //Pointer should be implemented in the derived class
};

template<typename Key, typename Value>
struct GeneralTreeNode : TreeNode<Key, Value> {
    GeneralTreeNode *left = nullptr;
    GeneralTreeNode *right = nullptr;
    GeneralTreeNode *parent = nullptr;
};

// Abstract Balanced Tree interface
template<typename Key, typename Value, typename Compare = std::less<Key>>
class BalancedTree {
    using Node = TreeNode<Key, Value>;

public:
    virtual ~BalancedTree() = default;
    virtual bool insert(const Key &key, const Value &value) = 0;
    virtual bool erase(const Key &key) = 0;
    virtual Value *find(const Key &key) = 0;
    virtual void clear() = 0;
    virtual std::size_t size() const = 0;
    virtual bool empty() const = 0;

    // For iteration
    virtual Node *minimum() = 0;
    virtual Node *next(Node *node) = 0;
    virtual Node *prev(Node *node) = 0;
};
#endif//ESTL_IBALANCEDTREE_HPP
