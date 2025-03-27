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
        const size_t DEFAULT_CAPACITY = 5000;
        MapType map;

        FixedMapTest(): map(DEFAULT_CAPACITY) {
        };
        void TearDown() override {
          map.clear(); // Ensure the map is cleared after each test
        }
    };

    template<std::size_t N>
    class FixedMapTest<CTMap<int, std::string, N>> : public ::testing::Test {
    protected:
        static const size_t DEFAULT_CAPACITY = 5000;
        CTMap<int, std::string, N> map;

        FixedMapTest(): map() {
            };
        void TearDown() override {
          map.clear(); // Ensure the map is cleared after each test
        }
    };

    template<std::size_t N>
    const std::size_t FixedMapTest<CTMap<int, std::string, N>>::DEFAULT_CAPACITY;

    using TestTypes = ::testing::Types<CTMap<int, std::string, 5000>, RTMap<int,
                                                                            std::string>>;
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

        CTMap<int, std::string, 10> map2;
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
      for (int i = 0; i < this->DEFAULT_CAPACITY ; ++i) {
        EXPECT_TRUE(this->map.insert(i, "value" + std::to_string(i)));
      }

      EXPECT_EQ(this->map.size(), this->DEFAULT_CAPACITY);

      // Attempt to exceed capacity
      EXPECT_FALSE(this->map.insert(this->DEFAULT_CAPACITY, "value" + std::to_string(this->DEFAULT_CAPACITY)));
      EXPECT_EQ(this->map.size(), this->DEFAULT_CAPACITY);
      EXPECT_EQ(this->map.find(this->DEFAULT_CAPACITY), nullptr);
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
  const int numThreads = 5;
  const int numOperations = 1000;

  auto insertTask = [this](int start, int end) {
    for (int i = start; i < end; ++i) {
      printf("insertTask %d\n", i);
      this->map.insert(i, "value" + std::to_string(i));
    }
  };

  auto eraseTask = [this](int start, int end) {
    for (int i = start; i < end; ++i) {
      printf("eraseTask %d\n", i);
      this->map.erase(i);
    }
  };


  this->map.clear();

  std::vector<std::thread> threads;

  // Launch threads to perform insertions
  for (int i = 0; i < numThreads; ++i) {
    threads.emplace_back(insertTask, i * numOperations, (i + 1) * numOperations);
  }

  // Wait for all insertion threads to finish
  for (auto &thread : threads) {
    thread.join();
  }

  EXPECT_EQ(this->map.size(), numThreads * numOperations);

  threads.clear();

  // Launch threads to perform deletions
  for (int i = 0; i < numThreads; ++i) {
    threads.emplace_back(eraseTask, i * numOperations, (i + 1) * numOperations);
  }

  // Wait for all deletion threads to finish
  for (auto &thread : threads) {
    thread.join();
  }

  EXPECT_EQ(this->map.size(), 0);
}
#endif

} // namespace ESTL
