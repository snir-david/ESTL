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
    template <typename MapType>
    class FixedMapTest : public ::testing::Test {
    protected:
        const size_t DEFAULT_CAPACITY = 4;
        MapType map;

        FixedMapTest(): map(DEFAULT_CAPACITY) {
        };
    };

    template<std::size_t N>
    class FixedMapTest<CTMap<int, std::string, N>> : public ::testing::Test {
    protected:
        static const size_t DEFAULT_CAPACITY = 4;
        CTMap<int, std::string, N> map;

        FixedMapTest(): map() {
            };
    };

    template<std::size_t N>
    const std::size_t FixedMapTest<CTMap<int, std::string, N>>::DEFAULT_CAPACITY;

    using TestTypes = ::testing::Types<FixedMap<int, std::string>, CTMap<int, std::string, 4>, RTMap<int, std::string>>;
    TYPED_TEST_SUITE(FixedMapTest, TestTypes);

    // Test constructor
    TYPED_TEST(FixedMapTest, Constructor) {
        EXPECT_EQ(this->map.size(), 0);
        EXPECT_TRUE(this->map.empty());
        EXPECT_EQ(this->map.capacity(), this->DEFAULT_CAPACITY);
    }

    // Test initializer list constructor
    TYPED_TEST(FixedMapTest, InitializerListConstructor) {
        TypeParam map = {{1, "one"}, {2, "two"}, {3, "three"}};
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
    TYPED_TEST(FixedMapTest, Insert) {
        EXPECT_TRUE(this->map.insert(1, "one"));
        EXPECT_TRUE(this->map.insert(2, "two"));
        EXPECT_EQ(this->map.size(), 2);
        EXPECT_EQ(*this->map.find(1), "one");
        EXPECT_EQ(*this->map.find(2), "two");

        // Insert duplicate key
        EXPECT_FALSE(this->map.insert(1, "one_duplicate"));
        EXPECT_EQ(*this->map.find(1), "one");// Original value unchanged
    }

    // Test erase
    TYPED_TEST(FixedMapTest, Erase) {
        EXPECT_TRUE(this->map.insert(1, "one"));
        EXPECT_TRUE(this->map.insert(2, "two"));
        EXPECT_TRUE(this->map.insert(3, "three"));
        EXPECT_EQ(this->map.size(), 3);
        EXPECT_TRUE(this->map.erase(2));
        EXPECT_EQ(this->map.size(), 2);
        EXPECT_EQ(this->map.find(2), nullptr);
        EXPECT_FALSE(this->map.erase(2));// Erase non-existent key
        EXPECT_EQ(this->map.size(), 2);
    }


    TYPED_TEST(FixedMapTest, InsertOrAssign) {
        EXPECT_TRUE(this->map.insert_or_assign(1, "one"));
        EXPECT_EQ(this->map[1], "one");

        // Assign a new value to the existing key
        EXPECT_FALSE(this->map.insert_or_assign(1, "uno"));
        EXPECT_EQ(this->map[1], "uno");
    }

    TYPED_TEST(FixedMapTest, Extract) {
        this->map.insert(1, "one");
        this->map.insert(2, "two");

        auto kvPair = this->map.extract(1);
        EXPECT_EQ(kvPair.first, 1);
        EXPECT_EQ(kvPair.second, "one");

        // Ensure the key is removed from the this->map
        EXPECT_THROW(this->map.extract(1), std::out_of_range);
        EXPECT_EQ(this->map.find(1), nullptr);
    }

    TYPED_TEST(FixedMapTest, Merge) {
        this->map.insert(1, "one");
        this->map.insert(2, "two");

        FixedMap<int, std::string> map2(10);
        map2.insert(3, "three");
        map2.insert(4, "four");

        this->map.merge(map2);

        EXPECT_EQ(this->map[1], "one");
        EXPECT_EQ(this->map[2], "two");
        EXPECT_EQ(this->map[3], "three");
        EXPECT_EQ(this->map[4], "four");

        // Ensure map2 is unchanged
        EXPECT_EQ(map2[3], "three");
        EXPECT_EQ(map2[4], "four");
    }

    // Test overflow
    TYPED_TEST(FixedMapTest, Overflow) {
        EXPECT_TRUE(this->map.insert(1, "one"));
        EXPECT_TRUE(this->map.insert(2, "two"));
        EXPECT_TRUE(this->map.insert(3, "three"));
        EXPECT_TRUE(this->map.insert(4, "four"));
        EXPECT_EQ(this->map.size(), 4);

        // Attempt to exceed capacity
        EXPECT_FALSE(this->map.insert(5, "five"));
        EXPECT_EQ(this->map.size(), 4);
        EXPECT_EQ(this->map.find(5), nullptr);
    }

    // Test underflow (erase from empty this->map)
    TYPED_TEST(FixedMapTest, Underflow) {
        EXPECT_FALSE(this->map.erase(1));// Empty this->map
        EXPECT_EQ(this->map.size(), 0);

        this->map.insert(1, "one");
        EXPECT_TRUE(this->map.erase(1));
        EXPECT_FALSE(this->map.erase(1));// Now empty again
        EXPECT_EQ(this->map.size(), 0);
    }

    // Test clear
    TYPED_TEST(FixedMapTest, Clear) {
        EXPECT_TRUE(this->map.insert(1, "one"));
        EXPECT_TRUE(this->map.insert(2, "two"));
        EXPECT_TRUE(this->map.insert(3, "three"));
        EXPECT_EQ(this->map.size(), 3);
        this->map.clear();
        EXPECT_EQ(this->map.size(), 0);
        EXPECT_TRUE(this->map.empty());
        EXPECT_EQ(this->map.find(1), nullptr);
        EXPECT_EQ(this->map.find(2), nullptr);
        EXPECT_EQ(this->map.find(3), nullptr);

        // Verify can insert after clear
        EXPECT_TRUE(this->map.insert(1, "one"));
        EXPECT_EQ(this->map.size(), 1);
    }

    // Test iterator
    TYPED_TEST(FixedMapTest, Iterator) {
        EXPECT_TRUE(this->map.insert(1, "one"));
        EXPECT_TRUE(this->map.insert(2, "two"));
        EXPECT_TRUE(this->map.insert(3, "three"));

        std::vector<std::pair<int, std::string>> result;
        for (const auto &pair: this->map) {
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
        auto it = this->map.end();
        --it;
        EXPECT_EQ(it->first, 3);
        --it;
        EXPECT_EQ(it->first, 2);
        ++it;
        EXPECT_EQ(it->first, 3);
    }

// Test multi-threading (requires ENABLE_THREAD_SAFETY to be true)
#if ENABLE_THREAD_SAFETY
    TYPED_TEST(FixedMapTest, MultiThreads) {
        const int NUM_THREADS = 4;
        std::vector<std::thread> threads;

        // Concurrent inserts
        threads.reserve(NUM_THREADS);
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back([this, i]() { this->map.insert(i, "value" + std::to_string(i)); });
        }

        for (auto &t: threads) {
            t.join();
        }

        EXPECT_EQ(this->map.size(), NUM_THREADS);
        for (int i = 0; i < NUM_THREADS; ++i) {
            EXPECT_EQ(*this->map.find(i), "value" + std::to_string(i));
        }

        // Concurrent erase and insert
        threads.clear();
        for (int i = 0; i < NUM_THREADS; ++i) {
            if (i % 2 == 0) {
                threads.emplace_back([this, i]() { this->map.erase(i); });
            } else {
                threads.emplace_back([this, i]() { this->map.insert(i + 10, "new" + std::to_string(i)); });
            }
        }

        for (auto &t: threads) {
            t.join();
        }

        EXPECT_EQ(this->map.find(0), nullptr);
        EXPECT_EQ(this->map.find(2), nullptr);
        EXPECT_EQ(*this->map.find(1), "value1");
        EXPECT_EQ(*this->map.find(3), "value3");
        EXPECT_EQ(*this->map.find(11), "new1");
        EXPECT_EQ(*this->map.find(13), "new3");
    }
#endif

}// namespace ESTL
