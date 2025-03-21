//
// Created by snirn on 3/20/25.
//

#ifndef FIXEDSET_HPP
#define FIXEDSET_HPP

#include "FixedUnorderedMap.hpp"

namespace ESTL {
    template<typename Key, typename Hash = std::hash<Key>>
    class FixedUnorderedSet {
        FixedUnorderedMap<Key, bool, Hash> *m_map;

    public:
        using Iterator = typename FixedUnorderedMap<Key, bool, Hash>::Iterator;


        explicit FixedUnorderedSet(FixedUnorderedMap<Key, bool, Hash> *map)
            : m_map(map) {}

        bool insert(const Key &key) { return m_map->insert(key, true); }

        bool erase(const Key &key) { return m_map->erase(key); }

        bool contains(const Key &key) const { return m_map->find(key) != nullptr; }

        void clear() { m_map->clear(); }

        std::size_t size() const { return m_map->size(); }

        bool empty() const { return m_map->empty(); }

        std::size_t capacity() const { return m_map->capacity(); }

        Iterator begin() { return m_map->begin(); }

        Iterator end() { return m_map->end(); }

        template<typename... Args>
        bool emplace(Args &&...args) {
            return m_map->insert(Key(std::forward<Args>(args)...), true);
        }

        template<typename InputIt>
        void insert_range(InputIt first, InputIt last) {
            for (auto it = first; it != last; ++it) {
                m_map->insert(*it, true);
            }
        }

        template<typename InputIt>
        void append_range(InputIt first, InputIt last) {
            for (auto it = first; it != last; ++it) {
                m_map->insert(*it, true);
            }
        }
    };

    // Compile-time fixed unordered map
    template<typename Key, std::size_t N, std::size_t BucketPoolSize = N / 2, typename Hash = std::hash<Key>>
    class CTUnorderedSet : public FixedUnorderedSet<Key, Hash> {
        CTMap<Key, bool, N, BucketPoolSize, Hash> m_map;

    public:
        CTUnorderedSet() : m_map() , FixedUnorderedSet<Key, Hash>(&m_map) {}

        CTUnorderedSet(std::initializer_list<const Key> initList)
            : CTUnorderedSet() {
            for (const auto &key: initList) {
                this->insert(key);
            }
        }
    };

    // Run-time fixed unordered map
    template<typename Key, typename Hash = std::hash<Key>>
    class RTUnorderedSet : public FixedUnorderedSet<Key, Hash> {
        RTMap<Key, bool, Hash> m_map;

    public:
        explicit RTUnorderedSet(std::size_t capacity, std::size_t poolSize = 0)
            : m_map(capacity, poolSize), FixedUnorderedSet<Key, Hash>(&m_map) {}

        RTUnorderedSet(std::initializer_list<const Key> initList, std::size_t capacity = 0, std::size_t poolSize = 0)
            : RTUnorderedSet(capacity ? capacity : initList.size(), poolSize) {
            for (const auto &key: initList) {
                this->insert(key);
            }
        }

        ~RTUnorderedSet() = default;
    };
}// namespace ESTL

#endif//FIXEDSET_HPP
