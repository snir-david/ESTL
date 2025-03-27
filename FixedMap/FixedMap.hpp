#pragma once

#include "BalancedTreeFactory.hpp"
#include "IBalancedTree.hpp"
#include <array>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>

#ifndef ENABLE_THREAD_SAFETY
#define ENABLE_THREAD_SAFETY true
#endif

namespace ESTL {
    // FixedMap class using BalancedTreeWP interface
    template<typename Key, typename Value, typename Compare = std::less<Key>>
    class FixedMap {
        using BalancedTreeWP = BalancedTree<Key, Value, Compare>;

    protected:
        std::unique_ptr<BalancedTreeWP> m_tree;
        std::size_t m_capacity;
#if ENABLE_THREAD_SAFETY
        mutable std::mutex m_mutex;
#endif

    public:
        FixedMap(TreeNode<Key, Value> *buffer, std::size_t capacity, TreeType treeType = TreeType::RedBlack)
            : m_capacity(capacity) {
            m_tree = BalancedTreeFactory<Key, Value, Compare>::createTree(treeType, buffer, capacity);
        }

        FixedMap(std::initializer_list<std::pair<const Key, Value>> initList, std::size_t capacity = 0, TreeType
                                                                                                                treeType = TreeType::RedBlack)
            : m_capacity(capacity ? capacity : initList.size()) {
            m_tree = BalancedTreeFactory<Key, Value, Compare>::createTree(treeType, m_capacity);
            for (const auto &pair: initList) {
                insert(pair.first, pair.second);
            }
        }

        void initFreeNodes() { m_tree->initFreeNodes(); }

        bool insert(const Key &key, const Value &value) {
#if ENABLE_THREAD_SAFETY
            std::lock_guard<std::mutex> lock(m_mutex);
#endif
            return m_tree->insert(key, value);
        }

        bool erase(const Key &key) {
#if ENABLE_THREAD_SAFETY
            std::lock_guard<std::mutex> lock(m_mutex);
#endif
            return m_tree->erase(key);
        }

        Value *find(const Key &key) {
#if ENABLE_THREAD_SAFETY
            std::lock_guard<std::mutex> lock(m_mutex);
#endif
            return m_tree->find(key);
        }

        Value &operator[](const Key &key) {
            Value *found = find(key);
            if (found)
                return *found;
            insert(key, Value());
            return *find(key);
        }

        void clear() {
#if ENABLE_THREAD_SAFETY
            std::lock_guard<std::mutex> lock(m_mutex);
#endif
            m_tree->clear();
        }

        // Insert or assign a key-value pair
        bool insert_or_assign(const Key &key, const Value &value) {
        #if ENABLE_THREAD_SAFETY
            std::lock_guard<std::mutex> lock(m_mutex);
        #endif
            Value *found = m_tree->find(key);
            if (found) {
                *found = value;
                return false;
            } else {
                return m_tree->insert(key, value);
            }
        }

        // Extract a key-value pair by key
        std::pair<Key, Value> extract(const Key &key) {
        #if ENABLE_THREAD_SAFETY
            std::lock_guard<std::mutex> lock(m_mutex);
        #endif
            Value *found = m_tree->find(key);
            if (!found) {
                throw std::out_of_range("Key not found");
            }
            std::pair<Key, Value> kvPair = {key, *found};
            m_tree->erase(key);
            return kvPair;
        }

        // Merge another FixedMap into this one
        void merge(FixedMap &other) {
        #if ENABLE_THREAD_SAFETY
            std::lock_guard<std::mutex> otherLock(other.m_mutex);
        #endif
            for (auto it = other.begin(); it != other.end(); ++it) {
                this->insert(it->first, it->second);
            }
        }

        std::size_t size() const {
#if ENABLE_THREAD_SAFETY
            std::lock_guard<std::mutex> lock(m_mutex);
#endif
            return m_tree->size();
        }

        bool empty() const {
#if ENABLE_THREAD_SAFETY
            std::lock_guard<std::mutex> lock(m_mutex);
#endif
            return m_tree->empty();
        }

        std::size_t capacity() const { return m_capacity; }

        class Iterator {
            using Node = TreeNode<Key, Value>;
        private:
            Node *m_current;
            BalancedTreeWP *m_tree;
            mutable std::pair<Key, Value> m_pair;// Store the pair as a member

        public:
            Iterator(Node *current, BalancedTreeWP *tree) : m_current(current), m_tree(tree) {}

            Iterator &operator++() {
                m_current = m_tree->next(m_current);
                return *this;
            }

            Iterator operator++(int) {
                Iterator temp = *this;
                m_current = m_tree->next(m_current);
                return temp;
            }

            Iterator &operator--() {
                m_current = m_tree->prev(m_current);
                return *this;
            }

            Iterator operator--(int) {
                Iterator temp = *this;
                m_current = m_tree->prev(m_current);
                return temp;
            }

            std::pair<const Key, Value> operator*() const {
                if (! m_current || ! m_current->in_use)
                    throw std::runtime_error("Dereferencing invalid iterator");
                return {m_current->key, m_current->value};
            }

            std::pair<Key, Value> *operator->() const {
                std::pair<const Key, Value> result = this->operator*();
                m_pair.first = result.first;
                m_pair.second = result.second;
                return &m_pair;
            }

            bool operator==(const Iterator &other) const { return m_current == other.m_current; }
            bool operator!=(const Iterator &other) const { return ! (*this == other); }
        };

        Iterator begin() { return Iterator(m_tree->minimum(), m_tree.get()); }
        Iterator end() { return Iterator(nullptr, m_tree.get()); }
    };


    // Compile-time fixed unordered map
    template<typename Key, typename Value, std::size_t N, typename Compare = std::less<Key>>
    class CTMap : public FixedMap<Key, Value, Compare> {

        std::array<TreeNode<Key, Value>, N> m_buckets;

    public:
        explicit CTMap(TreeType treeType = TreeType::RedBlack) : FixedMap<Key, Value, Compare>(m_buckets.data(), N,
                                            treeType) {
            this->initFreeNodes();
        }

        CTMap(std::initializer_list<std::pair<const Key, Value>> initList, TreeType treeType = TreeType::RedBlack)
            : CTMap(treeType) {
            for (const auto &item: initList) {
                this->insert(item.first, item.second);
            }
        }
    };

    // Run-time fixed unordered map
    template<typename Key, typename Value, typename Compare = std::less<Key>>
    class RTMap : public FixedMap<Key, Value, Compare> {
    TreeNode<Key, Value>* dynamicNodes;

    public:
        explicit RTMap(std::size_t capacity, TreeType treeType = TreeType::RedBlack) : FixedMap<Key, Value, Compare>(
                      dynamicNodes = new TreeNode<Key, Value>[capacity], capacity, treeType) {
            this->initFreeNodes();
        }

        RTMap(std::initializer_list<std::pair<const Key, Value>> initList, std::size_t capacity = 0 ,
              TreeType treeType = TreeType::RedBlack) : RTMap(capacity ? capacity : initList.size(), treeType) {
            for (const auto &item: initList) {
                this->insert(item.first, item.second);
            }
        }

        ~RTMap() { delete[] dynamicNodes; }
    };

}// namespace ESTL