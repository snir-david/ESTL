#include <gtest/gtest.h>
#include "../FixedVector.hpp"
#include <vector>
#include <thread>

namespace ESTL {
template<typename VectorType>
class FixedVectorTest : public ::testing::Test {
protected:
  std::size_t bufferSize = 10;
  VectorType vec;

  FixedVectorTest() : vec(bufferSize) {
    for (int i = 1; i <= 5; ++i) {
      vec.push_back(i);
    }
  }

  void createVectorWithValues() {
    VectorType vec1(bufferSize,{1,2,3,4,5});
    vec = vec1;
  }
};

template<std::size_t N>
class FixedVectorTest<CTVector<int, N> > : public ::testing::Test {
protected:
  static const std::size_t bufferSize = N;
  CTVector<int, N> vec;

  FixedVectorTest() : vec() {
    for (int i = 1; i <= std::min<std::size_t>(5, N); ++i) {
      vec.push_back(i);
    }
  }

  void createVectorWithValues() {
    CTVector<int, N> vec1{1,2,3,4,5};
    vec = vec1;
  }
};

template<std::size_t N>
const std::size_t FixedVectorTest<CTVector<int, N> >::bufferSize;

using TestTypes = ::testing::Types<RTVector<int>, CTVector<int, 10> >;
TYPED_TEST_SUITE(FixedVectorTest, TestTypes);

TYPED_TEST(FixedVectorTest, DefaultConstructor) {
  EXPECT_EQ(this->vec.size(), std::min<std::size_t>(5, this->bufferSize));
}

TYPED_TEST(FixedVectorTest, InitializerListConstructor) {
  this->createVectorWithValues();
  EXPECT_EQ(this->vec.size(), std::min<std::size_t>(5, this->bufferSize));
  EXPECT_FALSE(this->vec.empty());
  EXPECT_EQ(this->vec[0], 1);
  EXPECT_EQ(this->vec[this->vec.size() - 1], this->vec.size());
}

TYPED_TEST(FixedVectorTest, PushBackAndSize) {
  this->vec.clear();
  this->vec.push_back(42);
  this->vec.push_back(43);

  EXPECT_EQ(this->vec.size(), 2);
  EXPECT_EQ(this->vec[0], 42);
  EXPECT_EQ(this->vec[1], 43);
}

TYPED_TEST(FixedVectorTest, PopBack) {
  this->vec.clear();
  this->vec.push_back(1);
  this->vec.push_back(2);
  this->vec.push_back(3);
  this->vec.pop_back();

  EXPECT_EQ(this->vec.size(), 2);
  EXPECT_EQ(this->vec[0], 1);
  EXPECT_EQ(this->vec[1], 2);
}

TYPED_TEST(FixedVectorTest, PushBackOverflow) {
  this->vec.clear();
  for (size_t i = 0; i < this->bufferSize; ++i) {
    this->vec.push_back(static_cast<int>(i));
  }
  EXPECT_THROW(this->vec.push_back(99), std::out_of_range);
}

TYPED_TEST(FixedVectorTest, Emplace) {
  this->vec.clear();

  this->vec.emplace(this->vec.begin(), 1);
  this->vec.emplace(this->vec.begin() + 1, 2);
  this->vec.emplace(this->vec.begin() + 2, 3);

  EXPECT_EQ(this->vec.size(), 3);
  EXPECT_EQ(this->vec[0], 1);
  EXPECT_EQ(this->vec[1], 2);
  EXPECT_EQ(this->vec[2], 3);
}

TYPED_TEST(FixedVectorTest, EmplaceBack) {
  this->vec.clear();

  this->vec.emplace_back(1);
  this->vec.emplace_back(2);
  this->vec.emplace_back(3);

  EXPECT_EQ(this->vec.size(), 3);
  EXPECT_EQ(this->vec[0], 1);
  EXPECT_EQ(this->vec[1], 2);
  EXPECT_EQ(this->vec[2], 3);
}

TYPED_TEST(FixedVectorTest, AppendRange) {
  this->vec.clear();

  std::array<int, 3> range = {1, 2, 3};
  this->vec.append_range(range.begin(), range.end());

  EXPECT_EQ(this->vec.size(), 3);
  EXPECT_EQ(this->vec[0], 1);
  EXPECT_EQ(this->vec[1], 2);
  EXPECT_EQ(this->vec[2], 3);
}

TYPED_TEST(FixedVectorTest, PopBackUnderflow) {
  this->vec.clear();
  EXPECT_THROW(this->vec.pop_back(), std::out_of_range);
}

TYPED_TEST(FixedVectorTest, OperatorBracket) {
  this->vec.clear();
  this->vec.push_back(10);
  this->vec.push_back(20);
  this->vec.push_back(30);

  EXPECT_EQ(this->vec[0], 10);
  EXPECT_EQ(this->vec[1], 20);
  EXPECT_EQ(this->vec[2], 30);

  this->vec[1] = 25;
  EXPECT_EQ(this->vec[1], 25);

  EXPECT_THROW(this->vec[this->vec.size()], std::out_of_range);
}

TYPED_TEST(FixedVectorTest, At) {
  this->vec.clear();
  this->vec.push_back(10);
  this->vec.push_back(20);
  this->vec.push_back(30);

  EXPECT_EQ(this->vec.at(0), 10);
  EXPECT_EQ(this->vec.at(1), 20);
  EXPECT_EQ(this->vec.at(2), 30);

  this->vec.at(1) = 25;
  EXPECT_EQ(this->vec.at(1), 25);

  EXPECT_THROW(this->vec.at(this->vec.size()), std::out_of_range);
}

TYPED_TEST(FixedVectorTest, FrontAndBack) {
  this->vec.clear();
  this->vec.push_back(10);
  this->vec.push_back(20);
  this->vec.push_back(30);

  EXPECT_EQ(this->vec.front(), 10);
  EXPECT_EQ(this->vec.back(), 30);

  this->vec.front() = 15;
  this->vec.back() = 35;

  EXPECT_EQ(this->vec.front(), 15);
  EXPECT_EQ(this->vec.back(), 35);
}

TYPED_TEST(FixedVectorTest, Clear) {
  this->createVectorWithValues();
  this->vec.clear();

  EXPECT_EQ(this->vec.size(), 0);
  EXPECT_TRUE(this->vec.empty());
}

TYPED_TEST(FixedVectorTest, Insert) {
  this->vec.clear();
  this->vec.push_back(1);
  this->vec.push_back(2);
  this->vec.push_back(4);
  this->vec.push_back(5);

  auto it = this->vec.begin() + 2;
  this->vec.insert(it, 3);

  EXPECT_EQ(this->vec.size(), 5);
  EXPECT_EQ(this->vec[0], 1);
  EXPECT_EQ(this->vec[1], 2);
  EXPECT_EQ(this->vec[2], 3);
  EXPECT_EQ(this->vec[3], 4);
  EXPECT_EQ(this->vec[4], 5);
}

TYPED_TEST(FixedVectorTest, InsertAtBeginning) {
  this->vec.clear();
  this->vec.push_back(2);
  this->vec.push_back(3);
  this->vec.push_back(4);
  this->vec.insert(this->vec.begin(), 1);

  EXPECT_EQ(this->vec.size(), 4);
  EXPECT_EQ(this->vec[0], 1);
  EXPECT_EQ(this->vec[1], 2);
}

TYPED_TEST(FixedVectorTest, InsertAtEnd) {
  this->vec.clear();
  this->vec.push_back(1);
  this->vec.push_back(2);
  this->vec.push_back(3);
  this->vec.insert(this->vec.end(), 4);

  EXPECT_EQ(this->vec.size(), 4);
  EXPECT_EQ(this->vec[3], 4);
}

TYPED_TEST(FixedVectorTest, InsertOverflow) {
  this->vec.clear();
  for (size_t i = 0; i < this->bufferSize; ++i) {
    this->vec.push_back(static_cast<int>(i));
  }
  EXPECT_THROW(this->vec.insert(this->vec.begin(), 99), std::out_of_range);
}

TYPED_TEST(FixedVectorTest, Erase) {
  this->createVectorWithValues();
  auto it = this->vec.begin() + 2;
  this->vec.erase(it);

  EXPECT_EQ(this->vec.size(), 4);
  EXPECT_EQ(this->vec[0], 1);
  EXPECT_EQ(this->vec[1], 2);
  EXPECT_EQ(this->vec[2], 4);
  EXPECT_EQ(this->vec[3], 5);
}

TYPED_TEST(FixedVectorTest, EraseFirstElement) {
  this->vec.clear();
  this->vec.push_back(1);
  this->vec.push_back(2);
  this->vec.push_back(3);
  this->vec.push_back(4);
  this->vec.erase(this->vec.begin());

  EXPECT_EQ(this->vec.size(), 3);
  EXPECT_EQ(this->vec[0], 2);
  EXPECT_EQ(this->vec[1], 3);
  EXPECT_EQ(this->vec[2], 4);
}

TYPED_TEST(FixedVectorTest, EraseLastElement) {
  this->vec.clear();
  this->vec.push_back(1);
  this->vec.push_back(2);
  this->vec.push_back(3);
  this->vec.push_back(4);
  this->vec.erase(this->vec.end() - 1);

  EXPECT_EQ(this->vec.size(), 3);
  EXPECT_EQ(this->vec[0], 1);
  EXPECT_EQ(this->vec[1], 2);
  EXPECT_EQ(this->vec[2], 3);
}

TYPED_TEST(FixedVectorTest, EraseInvalidPosition) {
  this->createVectorWithValues();
  EXPECT_THROW(this->vec.erase(this->vec.end()), std::out_of_range);
}

TYPED_TEST(FixedVectorTest, Iterators) {
  this->createVectorWithValues();
  size_t i = 1;
  for (auto it = this->vec.begin(); it != this->vec.end(); ++it) {
    EXPECT_EQ(*it, i);
    ++i;
  }
}

// TYPED_TEST(FixedContainerTest, Swap) {
//     TypeParam vec1({1,2,3});
//     TypeParam vec2({4,5,6});
//
//     vec1.swap(vec2);
//
//     EXPECT_EQ(vec1.size(), 4);
//     EXPECT_EQ(vec2.size(), 3);
//
//     EXPECT_EQ(vec1[0], 4);
//     EXPECT_EQ(vec1[3], 7);
//
//     EXPECT_EQ(vec2[0], 1);
//     EXPECT_EQ(vec2[2], 3);
// }

#if (ENABLE_THREAD_SAFETY)
TYPED_TEST(FixedVectorTest, ThreadSafety) {
  this->vec.clear();
  std::vector<std::thread> threads;

  for (int i = 0; i < 5; ++i) {
    threads.emplace_back([this, i]() {
      for (int j = 0; j < 2; ++j) {
        this->vec.push_back(i * 10 + j);
      }
    });
  }

  for (auto &t: threads) {
    t.join();
  }

  EXPECT_EQ(this->vec.size(), 10);
}
#endif
}
