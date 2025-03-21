//
// Created by snirn on 3/18/25.
//
#include "../FixedUnorderedSet.hpp"
#include <gtest/gtest.h>
#include <thread>

namespace ESTL {
    template<typename SetType>
    class FixedUnorderedSetTest : public ::testing::Test {
    protected:
        std::size_t bufferSize = 10;
        SetType set;

        FixedUnorderedSetTest()
            : set(bufferSize) {}
    };

    template<std::size_t N>
    class FixedUnorderedSetTest<CTUnorderedSet<int, N>> : public ::testing::Test {
    protected:
        static const std::size_t bufferSize = N;
        CTUnorderedSet<int, N> set;

        FixedUnorderedSetTest() : set() {
        }
    };

    template<std::size_t N>
    const std::size_t FixedUnorderedSetTest<CTUnorderedSet<int, N>>::bufferSize;

    using TestTypes = ::testing::Types<CTUnorderedSet<int, 10>, RTUnorderedSet<int>>;
    TYPED_TEST_SUITE(FixedUnorderedSetTest, TestTypes);

    TYPED_TEST(FixedUnorderedSetTest, Constructor) {
        EXPECT_EQ(this->set.size(), 0);
        EXPECT_EQ(this->set.capacity(), this->bufferSize);
    }

    TYPED_TEST(FixedUnorderedSetTest, InitializerListConstructor) {
        TypeParam set = {1, 2};
        EXPECT_TRUE(set.contains(1));
        EXPECT_TRUE(set.contains(2));
    }

    TYPED_TEST(FixedUnorderedSetTest, InsertAndcontains) {
        EXPECT_TRUE(this->set.insert(1));
        EXPECT_TRUE(this->set.insert(2));
        EXPECT_TRUE(this->set.contains(1));
        EXPECT_TRUE(this->set.contains(2));
        EXPECT_FALSE(this->set.contains(3));
    }

    TYPED_TEST(FixedUnorderedSetTest, InsertDuplicate) {
        EXPECT_TRUE(this->set.insert(1));
        EXPECT_FALSE(this->set.insert(1));// Duplicate key
        EXPECT_TRUE(this->set.contains(1));
    }

    TYPED_TEST(FixedUnorderedSetTest, Erase) {
        this->set.insert(1);
        this->set.insert(2);
        EXPECT_TRUE(this->set.erase(1));
        EXPECT_FALSE(this->set.contains(1));
        EXPECT_TRUE(this->set.contains(2));
        EXPECT_FALSE(this->set.erase(3));// Non-existent key
    }

    TYPED_TEST(FixedUnorderedSetTest, Overflow) {
        for (std::size_t i = 0; i < this->bufferSize + this->bufferSize / 2; ++i) {
            EXPECT_TRUE(this->set.insert(i));
        }
        EXPECT_THROW(this->set.insert(this->bufferSize + this->bufferSize / 2 + 1), std::out_of_range);
    }

    TYPED_TEST(FixedUnorderedSetTest, Underflow) {
        EXPECT_FALSE(this->set.erase(1));// Erase from empty set
    }

    TYPED_TEST(FixedUnorderedSetTest, Clear) {
        this->set.insert(1);
        this->set.insert(2);
        this->set.clear();
        EXPECT_EQ(this->set.size(), 0);
        EXPECT_FALSE(this->set.contains(1));
        EXPECT_FALSE(this->set.contains(2));
    }

    //    TYPED_TEST(FixedUnorderedSetTest, Iterator) {
    //        this->set.insert(1);
    //        this->set.insert(2);
    //        std::vector<std::pair<int, std::string> > elements;
    //        for (auto it = this->set.begin(); it != this->set.end(); ++it) {
    //            elements.push_back(it);
    //        }
    //        EXPECT_EQ(elements.size(), 2);
    //    }

    TYPED_TEST(FixedUnorderedSetTest, MultiThreads) {
        std::vector<std::thread> threads;
        for (int i = 0; i < 10; ++i) {
            threads.emplace_back([this, i]() { this->set.insert(i); });
        }
        for (auto &t: threads) {
            t.join();
        }
        EXPECT_EQ(this->set.size(), 10);
    }
}// namespace ESTL
