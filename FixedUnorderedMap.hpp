//
// Created by snirn on 3/18/25.
//
#pragma once

#include <array>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <vector>

#ifndef ENABLE_THREAD_SAFETY
#define ENABLE_THREAD_SAFETY true
#endif

namespace ESTL {
    enum class ProbingStrategy { CHAINING, LINEAR_PROBING, QUADRATIC_PROBING };

    template<typename K, typename H>
    class FixedUnorderedSet;
    template<typename K, std::size_t M, std::size_t P, typename H>
    class CTUnorderedSet;
    template<typename K, typename H>
    class RTUnorderedSet;

    template<typename Key, typename Value, typename Hash = std::hash<Key>>
    class FixedUnorderedMap {
        template<typename K, typename H>
        friend class FixedUnorderedSet;
        template<typename K, std::size_t M, std::size_t P, typename H>
        friend class CTUnorderedSet;
        template<typename K, typename H>
        friend class RTUnorderedSet;

    protected:
        struct Bucket {
            Key key;
            Value value;
            bool occupied = false;
            Bucket *next = nullptr;// Chaining for collisions
        };

        // Gets the first available free bucket
        Bucket *getFreeBucket() {
            if (! m_freeBuckets) {
                throw std::out_of_range("No more free buckets available");
            }
            Bucket *bucket = m_freeBuckets;
            m_freeBuckets = m_freeBuckets->next;
            bucket->next = nullptr;
            return bucket;
        }

        // Returns a bucket to the free pool
        void returnBucket(Bucket *bucket) {
            bucket->occupied = false;
            bucket->next = m_freeBuckets;
            m_freeBuckets = bucket;
        }

        std::size_t getBucketIndex(const Key &key) const { return m_hasher(key) % m_mapCapacity; }

        Bucket *m_buckets;
        Bucket *m_bucketPool;
        Bucket *m_freeBuckets;// Pool of available buckets
        Hash m_hasher;
        std::size_t m_size;
        std::size_t m_mapCapacity;
        std::size_t m_bucketPoolCapacity;
        ProbingStrategy m_probingStrategy;
#if (ENABLE_THREAD_SAFETY)
        mutable std::mutex m_mutex;
#endif

    public:
        FixedUnorderedMap(Bucket *bucketBuffer, Bucket *bucketPool, size_t mapCapacity, size_t poolCapacity)
            : m_buckets(bucketBuffer)
            , m_bucketPool(bucketPool)
            , m_freeBuckets(nullptr)
            , m_size(0)
            , m_mapCapacity(mapCapacity)
            , m_bucketPoolCapacity(poolCapacity)
            , m_probingStrategy(ProbingStrategy::CHAINING) {
            initFreeBucketPool();
        }

        // Helper to initialize the free bucket pool
        void initFreeBucketPool() {
            for (std::size_t i = 0; i < m_bucketPoolCapacity - 1; ++i) {
                m_bucketPool[i].next = &m_bucketPool[i + 1];
            }
            m_bucketPool[m_bucketPoolCapacity - 1].next = nullptr;
            m_freeBuckets = &m_bucketPool[0];
        }

        bool insert(const Key &key, const Value &value) {
            std::size_t index = getBucketIndex(key);
            Bucket *bucket = &m_buckets[index];
            // If bucket is free, use it
#if (ENABLE_THREAD_SAFETY)
            std::lock_guard<std::mutex> lock(m_mutex);
#endif
            if (! bucket->occupied) {
                bucket->key = key;
                bucket->value = value;
                bucket->occupied = true;
                ++m_size;
            } else if (bucket->key == key) {
                return false;// Key already exists
            } else {
                // Collision handling
                // Separate chaining
                while (bucket->next) {
                    bucket = bucket->next;
                    if (bucket->key == key) {
                        return false;// Key already exists
                    }
                }
                // Allocate new bucket
                Bucket *newBucket = getFreeBucket();
                newBucket->key = key;
                newBucket->value = value;
                newBucket->occupied = true;
                bucket->next = newBucket;
                ++m_size;
            }
            return true;
        }

        // Insert or assign method
        bool insert_or_assign(const Key &key, const Value &value) {
            std::size_t index = getBucketIndex(key);
            Bucket *bucket = &m_buckets[index];
#if (ENABLE_THREAD_SAFETY)
            std::lock_guard<std::mutex> lock(m_mutex);
#endif
            if (! bucket->occupied) {
                bucket->key = key;
                bucket->value = value;
                bucket->occupied = true;
                ++m_size;
                return true;
            } else if (bucket->key == key) {
                bucket->value = value;
                return false;
            } else {
                while (bucket->next) {
                    bucket = bucket->next;
                    if (bucket->key == key) {
                        bucket->value = value;
                        return false;
                    }
                }
                Bucket *newBucket = getFreeBucket();
                newBucket->key = key;
                newBucket->value = value;
                newBucket->occupied = true;
                bucket->next = newBucket;
                ++m_size;
                return true;
            }
        }

        bool erase(const Key &key) {
            std::size_t index = getBucketIndex(key);
            Bucket *bucket = &m_buckets[index];
            Bucket *prev = nullptr;
#if (ENABLE_THREAD_SAFETY)
            std::lock_guard<std::mutex> lock(m_mutex);
#endif
            while (bucket) {
                if (bucket->occupied && bucket->key == key) {
                    if (prev) {
                        prev->next = bucket->next;
                        returnBucket(bucket);
                    } else {
                        if (bucket->next) {
                            // Move next bucket data into current bucket
                            Bucket *nextBucket = bucket->next;
                            *bucket = *nextBucket;
                            returnBucket(nextBucket);
                        } else {
                            bucket->occupied = false;
                        }
                    }
                    --m_size;
                    return true;
                }
                prev = bucket;
                bucket = bucket->next;
            }
            return false;
        }

        Value *find(const Key &key) const {
            std::size_t index = getBucketIndex(key);
#if (ENABLE_THREAD_SAFETY)
            std::lock_guard<std::mutex> lock(m_mutex);
#endif
            Bucket *bucket = &m_buckets[index];
            while (bucket) {
                if (bucket->occupied && bucket->key == key) {
                    return &bucket->value;
                }
                bucket = bucket->next;
            }
            return nullptr;
        }

        Value &operator[](const Key &key) {
#if (ENABLE_THREAD_SAFETY)
            std::lock_guard<std::mutex> lock(m_mutex);
#endif
            Value *found = find(key);
            if (found)
                return *found;

            // Insert if not found
            insert(key, Value());
            return *find(key);
        }

        void clear() {
#if (ENABLE_THREAD_SAFETY)
            std::lock_guard<std::mutex> lock(m_mutex);
#endif
            for (std::size_t i = 0; i < m_mapCapacity; ++i) {
                Bucket *bucket = &m_buckets[i];
                if (bucket->occupied) {
                    bucket->occupied = false;

                    // Handle chained buckets
                    Bucket *current = bucket->next;
                    while (current) {
                        Bucket *next = current->next;
                        returnBucket(current);
                        current = next;
                    }
                    bucket->next = nullptr;
                }
            }
            m_size = 0;
        }

        // Extract a key-value pair from the map
        std::pair<Key, Value> extract(const Key &key) {
#if (ENABLE_THREAD_SAFETY)
            std::lock_guard<std::mutex> lock(m_mutex);
#endif
            std::size_t index = getBucketIndex(key);
            Bucket *bucket = &m_buckets[index];
            Bucket *prev = nullptr;

            while (bucket) {
                if (bucket->occupied && bucket->key == key) {
                    std::pair<Key, Value> kvPair = {bucket->key, bucket->value};
                    if (prev) {
                        prev->next = bucket->next;
                        returnBucket(bucket);
                    } else {
                        if (bucket->next) {
                            Bucket *nextBucket = bucket->next;
                            *bucket = *nextBucket;
                            returnBucket(nextBucket);
                        } else {
                            bucket->occupied = false;
                        }
                    }
                    --m_size;
                    return kvPair;
                }
                prev = bucket;
                bucket = bucket->next;
            }
            throw std::out_of_range("Key not found");
        }

        // Merge another FixedUnorderedMap into this one
        void merge(FixedUnorderedMap &other) {
#if (ENABLE_THREAD_SAFETY)
            std::lock_guard<std::mutex> otherLock(other.m_mutex);
#endif
            for (std::size_t i = 0; i < other.m_mapCapacity; ++i) {
                Bucket *bucket = &other.m_buckets[i];
                while (bucket && bucket->occupied) {
                    this->insert(bucket->key, bucket->value);
                    bucket = bucket->next;
                }
            }
        }

        std::size_t size() const {
#if (ENABLE_THREAD_SAFETY)
            std::lock_guard<std::mutex> lock(m_mutex);
#endif
            return m_size;
        }

        bool empty() const {
#if (ENABLE_THREAD_SAFETY)
            std::lock_guard<std::mutex> lock(m_mutex);
#endif
            return m_size == 0;
        }

        std::size_t capacity() const { return m_mapCapacity; }

        class Iterator {
        private:
            Bucket *m_current;         // Current primary bucket
            Bucket *m_chainCurrent;    // Current position in chain (could be primary bucket or a chained bucket)
            Bucket *m_buckets;         // Start of buckets array
            std::size_t m_mapCapacity; // Total number of primary buckets
            std::size_t m_currentIndex;// Current index in primary buckets array

            // Find the next valid bucket (occupied)
            void findNextValid() {
                // First check if we're in a chain and can move to next in chain
                if (m_chainCurrent && m_chainCurrent->next) {
                    m_chainCurrent = m_chainCurrent->next;
                    return;
                }

                // Otherwise, move to the next primary bucket
                m_chainCurrent = nullptr;
                m_currentIndex++;

                // Keep advancing until we find an occupied bucket or reach the end
                while (m_currentIndex < m_mapCapacity) {
                    m_current = &m_buckets[m_currentIndex];

                    if (m_current->occupied) {
                        m_chainCurrent = m_current;
                        return;
                    }

                    m_currentIndex++;
                }

                // If we get here, we've reached the end
                m_current = nullptr;
                m_chainCurrent = nullptr;
            }

        public:
            // Iterator typedefs for STL compatibility
            using value_type = std::pair<const Key &, Value &>;
            using difference_type = std::ptrdiff_t;
            using pointer = value_type *;
            using reference = value_type &;
            using iterator_category = std::forward_iterator_tag;

            // Constructor
            Iterator(Bucket *buckets, std::size_t mapCapacity, std::size_t startIndex = 0)
                : m_buckets(buckets)
                , m_mapCapacity(mapCapacity)
                , m_currentIndex(startIndex)
                , m_current(nullptr)
                , m_chainCurrent(nullptr) {
                // Handle empty map
                if (m_mapCapacity == 0 || ! m_buckets) {
                    m_current = nullptr;
                    m_chainCurrent = nullptr;
                    return;
                }

                // Start at the first bucket
                m_current = &m_buckets[m_currentIndex];

                // If first bucket is not occupied, find the first valid one
                if (! m_current->occupied) {
                    m_currentIndex--;// Decrement so findNextValid will start at index 0
                    findNextValid();
                } else {
                    m_chainCurrent = m_current;
                }
            }

            // Pre-increment operator
            Iterator &operator++() {
                if (m_chainCurrent) {
                    findNextValid();
                }
                return *this;
            }

            // Post-increment operator
            Iterator operator++(int) {
                Iterator temp = *this;
                ++(*this);
                return temp;
            }

            // Dereference operator
            std::pair<const Key &, Value &> operator*() const {
                if (! m_chainCurrent) {
                    throw std::runtime_error("Dereferencing invalid iterator");
                }
                return {m_chainCurrent->key, m_chainCurrent->value};
            }

            // Arrow operator
            std::pair<const Key &, Value &> *operator->() const {
                static std::pair<const Key &, Value &> result = this->operator*();
                return &result;
            }

            // Equality operator
            bool operator==(const Iterator &other) const { return m_chainCurrent == other.m_chainCurrent; }

            // Inequality operator
            bool operator!=(const Iterator &other) const { return ! (*this == other); }
        };

        // Begin iterator
        Iterator begin() { return Iterator(m_buckets, m_mapCapacity, 0); }

        // End iterator
        Iterator end() { return Iterator(m_buckets, m_mapCapacity, m_mapCapacity); }
    };

    // Compile-time fixed unordered map
    template<typename Key, typename Value, std::size_t N, std::size_t BucketPoolSize = N / 2,
             typename Hash = std::hash<Key>>
    class CTMap : public FixedUnorderedMap<Key, Value, Hash> {
        template<typename K, std::size_t M, std::size_t P, typename H>
        friend class CTUnorderedSet;

        std::array<typename FixedUnorderedMap<Key, Value, Hash>::Bucket, N> m_buckets;
        std::array<typename FixedUnorderedMap<Key, Value, Hash>::Bucket, BucketPoolSize> m_bucketPool;

    public:
        CTMap()
            : FixedUnorderedMap<Key, Value, Hash>(m_buckets.data(), m_bucketPool.data(), N, BucketPoolSize) {
            this->initFreeBucketPool();
        }

        CTMap(std::initializer_list<std::pair<const Key, Value>> initList)
            : CTMap() {
            for (const auto &item: initList) {
                this->insert(item.first, item.second);
            }
        }
    };

    // Run-time fixed unordered map
    template<typename Key, typename Value, typename Hash = std::hash<Key>>
    class RTMap : public FixedUnorderedMap<Key, Value, Hash> {
    public:
        explicit RTMap(std::size_t capacity, std::size_t poolSize = 0)
            : FixedUnorderedMap<Key, Value, Hash>(
                      new typename FixedUnorderedMap<Key, Value, Hash>::Bucket[capacity],
                      new typename FixedUnorderedMap<Key, Value, Hash>::Bucket[poolSize != 0 ? poolSize : capacity / 2],
                      capacity, poolSize != 0 ? poolSize : capacity / 2) {
            this->initFreeBucketPool();
        }

        RTMap(std::initializer_list<std::pair<const Key, Value>> initList, std::size_t capacity = 0,
              std::size_t poolSize = 0)
            : RTMap(capacity ? capacity : initList.size(), poolSize) {
            for (const auto &item: initList) {
                this->insert(item.first, item.second);
            }
        }

        ~RTMap() {
            delete[] this->m_buckets;
            delete[] this->m_bucketPool;
        }
    };
}// namespace ESTL
