//
// Created by SnirN on 3/20/2025.
//
#include "../FixedMap/FixedMap.hpp"// Adjust the include path as needed
#include <algorithm>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

namespace ESTL {

    // Test fixture for FixedMap tests
    class FixedMapTest : public ::testing::TestWithParam<TreeType> {
    protected:
        const size_t DEFAULT_CAPACITY = 4;
        FixedMap<int, std::string> map;

        FixedMapTest(): map(DEFAULT_CAPACITY, GetParam()) {
        };
    };

    // Test constructor
    TEST_P(FixedMapTest, Constructor) {
        EXPECT_EQ(map.size(), 0);
        EXPECT_TRUE(map.empty());
        EXPECT_EQ(map.capacity(), this->DEFAULT_CAPACITY);
    }

    // Test initializer list constructor
    TEST_P(FixedMapTest, InitializerListConstructor) {
        FixedMap<int, std::string> map({{1, "one"}, {2, "two"}, {3, "three"}} , this->DEFAULT_CAPACITY, GetParam());
        EXPECT_EQ(map.size(), 3);
        EXPECT_FALSE(map.empty());
        EXPECT_EQ(*map.find(1), "one");
        EXPECT_EQ(*map.find(2), "two");
        EXPECT_EQ(*map.find(3), "three");

        // Verify sorted order via iterator
        std::vector<int> keys;
        for (const auto &pair: map) {
            keys.push_back(pair.first);
        }
        EXPECT_TRUE(std::is_sorted(keys.begin(), keys.end()));
    }

    // Test insert
    TEST_P(FixedMapTest, Insert) {
        EXPECT_TRUE(map.insert(1, "one"));
        EXPECT_TRUE(map.insert(2, "two"));
        EXPECT_EQ(map.size(), 2);
        EXPECT_EQ(*map.find(1), "one");
        EXPECT_EQ(*map.find(2), "two");

        // Insert duplicate key
        EXPECT_FALSE(map.insert(1, "one_duplicate"));
        EXPECT_EQ(*map.find(1), "one");// Original value unchanged
    }

    // Test erase
    TEST_P(FixedMapTest, Erase) {
        EXPECT_TRUE(map.insert(1, "one"));
        EXPECT_TRUE(map.insert(2, "two"));
        EXPECT_TRUE(map.insert(3, "three"));
        EXPECT_EQ(map.size(), 3);
        EXPECT_TRUE(map.erase(2));
        EXPECT_EQ(map.size(), 2);
        EXPECT_EQ(map.find(2), nullptr);
        EXPECT_FALSE(map.erase(2));// Erase non-existent key
        EXPECT_EQ(map.size(), 2);
    }


    TEST_P(FixedMapTest, InsertOrAssign) {
        EXPECT_TRUE(map.insert_or_assign(1, "one"));
        EXPECT_EQ(map[1], "one");

        // Assign a new value to the existing key
        EXPECT_FALSE(map.insert_or_assign(1, "uno"));
        EXPECT_EQ(map[1], "uno");
    }

    TEST_P(FixedMapTest, Extract) {
        map.insert(1, "one");
        map.insert(2, "two");

        auto kvPair = map.extract(1);
        EXPECT_EQ(kvPair.first, 1);
        EXPECT_EQ(kvPair.second, "one");

        // Ensure the key is removed from the map
        EXPECT_THROW(map.extract(1), std::out_of_range);
        EXPECT_EQ(map.find(1), nullptr);
    }

    TEST_P(FixedMapTest, Merge) {
        map.insert(1, "one");
        map.insert(2, "two");

        FixedMap<int, std::string> map2(10);
        map2.insert(3, "three");
        map2.insert(4, "four");

        map.merge(map2);

        EXPECT_EQ(map[1], "one");
        EXPECT_EQ(map[2], "two");
        EXPECT_EQ(map[3], "three");
        EXPECT_EQ(map[4], "four");

        // Ensure map2 is unchanged
        EXPECT_EQ(map2[3], "three");
        EXPECT_EQ(map2[4], "four");
    }

    // Test overflow
    TEST_P(FixedMapTest, Overflow) {
        EXPECT_TRUE(map.insert(1, "one"));
        EXPECT_TRUE(map.insert(2, "two"));
        EXPECT_TRUE(map.insert(3, "three"));
        EXPECT_TRUE(map.insert(4, "four"));
        EXPECT_EQ(map.size(), 4);

        // Attempt to exceed capacity
        EXPECT_FALSE(map.insert(5, "five"));
        EXPECT_EQ(map.size(), 4);
        EXPECT_EQ(map.find(5), nullptr);
    }

    // Test underflow (erase from empty map)
    TEST_P(FixedMapTest, Underflow) {
        EXPECT_FALSE(map.erase(1));// Empty map
        EXPECT_EQ(map.size(), 0);

        map.insert(1, "one");
        EXPECT_TRUE(map.erase(1));
        EXPECT_FALSE(map.erase(1));// Now empty again
        EXPECT_EQ(map.size(), 0);
    }

    // Test clear
    TEST_P(FixedMapTest, Clear) {
        EXPECT_TRUE(map.insert(1, "one"));
        EXPECT_TRUE(map.insert(2, "two"));
        EXPECT_TRUE(map.insert(3, "three"));
        EXPECT_EQ(map.size(), 3);
        map.clear();
        EXPECT_EQ(map.size(), 0);
        EXPECT_TRUE(map.empty());
        EXPECT_EQ(map.find(1), nullptr);
        EXPECT_EQ(map.find(2), nullptr);
        EXPECT_EQ(map.find(3), nullptr);

        // Verify can insert after clear
        EXPECT_TRUE(map.insert(1, "one"));
        EXPECT_EQ(map.size(), 1);
    }

    // Test iterator
    TEST_P(FixedMapTest, Iterator) {
        EXPECT_TRUE(map.insert(1, "one"));
        EXPECT_TRUE(map.insert(2, "two"));
        EXPECT_TRUE(map.insert(3, "three"));

        std::vector<std::pair<int, std::string>> result;
        for (const auto &pair: map) {
            result.emplace_back(pair.first, pair.second);
        }

        EXPECT_EQ(result.size(), 3);
        EXPECT_EQ(result[0].first, 1);
        EXPECT_EQ(result[0].second, "one");
        EXPECT_EQ(result[1].first, 2);
        EXPECT_EQ(result[1].second, "two");
        EXPECT_EQ(result[2].first, 3);
        EXPECT_EQ(result[2].second, "three");

        // Test bidirectional iterator
        auto it = map.end();
        --it;
        EXPECT_EQ(it->first, 3);
        --it;
        EXPECT_EQ(it->first, 2);
        ++it;
        EXPECT_EQ(it->first, 3);
    }

// Test multi-threading (requires ENABLE_THREAD_SAFETY to be true)
#if ENABLE_THREAD_SAFETY
    TEST_P(FixedMapTest, MultiThreads) {
        const int NUM_THREADS = 4;
        std::vector<std::thread> threads;

        // Concurrent inserts
        threads.reserve(NUM_THREADS);
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back([this, i]() { map.insert(i, "value" + std::to_string(i)); });
        }

        for (auto &t: threads) {
            t.join();
        }

        EXPECT_EQ(map.size(), NUM_THREADS);
        for (int i = 0; i < NUM_THREADS; ++i) {
            EXPECT_EQ(*map.find(i), "value" + std::to_string(i));
        }

        // Concurrent erase and insert
        threads.clear();
        for (int i = 0; i < NUM_THREADS; ++i) {
            if (i % 2 == 0) {
                threads.emplace_back([this, i]() { map.erase(i); });
            } else {
                threads.emplace_back([this, i]() { map.insert(i + 10, "new" + std::to_string(i)); });
            }
        }

        for (auto &t: threads) {
            t.join();
        }

        EXPECT_EQ(map.find(0), nullptr);
        EXPECT_EQ(map.find(2), nullptr);
        EXPECT_EQ(*map.find(1), "value1");
        EXPECT_EQ(*map.find(3), "value3");
        EXPECT_EQ(*map.find(11), "new1");
        EXPECT_EQ(*map.find(13), "new3");
    }
#endif
INSTANTIATE_TEST_SUITE_P(TreeTypes, FixedMapTest, ::testing::Values(TreeType::RedBlack, TreeType::AVL));

}// namespace ESTL
