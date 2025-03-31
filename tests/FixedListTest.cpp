#include <gtest/gtest.h>
#include "../FixedList.hpp"
#include <thread>

namespace ESTL {
// Define a test fixture template
template<typename ListType>
class FixedListTest : public ::testing::Test {
protected:
  std::size_t bufferSize = 10;
  ListType list;

  FixedListTest() : list(bufferSize) {
    // Works for RTList
    for (int i = 1; i <= 5; ++i) {
      list.push_back(i);
    }
  }
};

// Specialization for CTList (Compile-time Fixed List)
template<std::size_t N>
class FixedListTest<CTList<int, N> > : public ::testing::Test {
protected:
  CTList<int, N> list;

  FixedListTest() {
    for (int i = 1; i <= 5; ++i) {
      list.push_back(i);
    }
  }
};

// Register the types to be tested
using TestTypes = ::testing::Types<RTList<int>, CTList<int, 10> >;
TYPED_TEST_SUITE(FixedListTest, TestTypes);

// ** Unified Test Cases for Both List Types **
// Constructor Test
TYPED_TEST(FixedListTest, Constructor) {
  EXPECT_EQ(this->list.size(), 5);
  EXPECT_EQ(this->list.capacity(), 10);
}

TEST(FixedListTest, ListInitConstructor) {
  CTList<int, 10> list1{1, 2, 3, 4, 5};
  RTList<int> list2(10, {1, 2, 3, 4, 5});
  auto it1 = list1.begin(), it2 = list2.begin();
  for (int i = 1; i <= 5; ++i, it1++, it2++) {
    EXPECT_EQ(*it1, *it2);
  }
  EXPECT_EQ(list1.capacity(), 10);
  EXPECT_EQ(list1.size(), 5);
  EXPECT_EQ(list2.capacity(), 10);
  EXPECT_EQ(list2.size(), 5);
}

// Push & Pop Back
TYPED_TEST(FixedListTest, PushPopBack) {
  this->list.push_back(10);
  EXPECT_EQ(this->list.back(), 10);
  this->list.pop_back();
  EXPECT_EQ(this->list.back(), 5);
}

// Push & Pop Front
TYPED_TEST(FixedListTest, PushPopFront) {
  this->list.push_front(20);
  EXPECT_EQ(this->list.front(), 20);
  this->list.pop_front();
  EXPECT_EQ(this->list.front(), 1);
}

// Insert at Specific Position
TYPED_TEST(FixedListTest, InsertAtPosition) {
  auto it = this->list.begin();
  std::advance(it, 2);
  this->list.insert(it, 99);

  it = this->list.begin();
  std::advance(it, 2);
  EXPECT_EQ(*it, 99);
  EXPECT_EQ(this->list.size(), 6);
}

// Emplace Back
TYPED_TEST(FixedListTest, EmplaceBack) {
  this->list.emplace_back(10);
  EXPECT_EQ(this->list.back(), 10);
  EXPECT_EQ(this->list.size(), 6);
}

// Emplace Front
TYPED_TEST(FixedListTest, EmplaceFront) {
  this->list.emplace_front(20);
  EXPECT_EQ(this->list.front(), 20);
  EXPECT_EQ(this->list.size(), 6);
}

// Emplace at Specific Position
TYPED_TEST(FixedListTest, EmplaceAtPosition) {
  auto it = this->list.begin();
  std::advance(it, 2);
  this->list.emplace(it, 99);

  it = this->list.begin();
  std::advance(it, 2);
  EXPECT_EQ(*it, 99);
  EXPECT_EQ(this->list.size(), 6);
}

// Erase at Specific Position
TYPED_TEST(FixedListTest, EraseAtPosition) {
  auto it = this->list.begin();
  std::advance(it, 2);
  this->list.erase(it);

  it = this->list.begin();
  std::advance(it, 2);
  EXPECT_NE(*it, 3); // 3 should be removed
  EXPECT_EQ(this->list.size(), 4);
}

// Full Capacity Handling (Overflow)
TYPED_TEST(FixedListTest, FullCapacityHandling) {
  for (int i = 6; i <= this->list.capacity(); ++i) {
    this->list.push_back(i);
  }
  EXPECT_TRUE(this->list.full());
  EXPECT_THROW(this->list.push_back(11), std::out_of_range);
}

TYPED_TEST(FixedListTest, ClearAndUnderFlowHandling) {
  this->list.clear();
  EXPECT_TRUE(this->list.empty());
  EXPECT_THROW(this->list.front(), std::out_of_range);
}

TYPED_TEST(FixedListTest, Iterator) {
  size_t i = 1;
  for (auto it = this->list.begin(); it != this->list.end(); ++it) {
    EXPECT_EQ(*it, i);
    ++i;
  }
}


// Merge Test
TYPED_TEST(FixedListTest, Merge) {
  CTList<int, 10> otherList;
  otherList.push_back(6);
  otherList.push_back(7);

  this->list.merge(otherList);

  EXPECT_EQ(this->list.size(), 7);
  EXPECT_TRUE(otherList.empty());
}

// Splice Test
TYPED_TEST(FixedListTest, Splice) {
  CTList<int, 10> otherList;
  otherList.push_back(6);
  otherList.push_back(7);

  auto it = this->list.begin();
  std::advance(it, 2);
  this->list.splice(it, otherList);

  EXPECT_EQ(this->list.size(), 7);
  EXPECT_TRUE(otherList.empty());
}

// Remove Test
TYPED_TEST(FixedListTest, Remove) {
  this->list.push_back(3);
  this->list.remove(3);

  for (auto& elem : this->list) {
    EXPECT_NE(elem, 3);
  }
}

// Remove If Test
TYPED_TEST(FixedListTest, RemoveIf) {
  this->list.push_back(3);
  this->list.push_back(4);
  this->list.push_back(5);
  this->list.remove_if([](const int& value) { return value % 2 == 0; });

  for (auto& elem : this->list) {
    EXPECT_FALSE(elem % 2 == 0);
  }
  EXPECT_EQ(this->list.size(), 5);
}

// Unique Test
TYPED_TEST(FixedListTest, Unique) {
  this->list.push_back(3);
  this->list.push_back(3);
  this->list.unique();

  auto it = this->list.begin();
  while (it != this->list.end()) {
    auto next = std::next(it);
    if (next != this->list.end()) {
      EXPECT_NE(*it, *next);
    }
    ++it;
  }

  EXPECT_EQ(this->list.size(), 6);
}

// Thread Safety Test (If Enabled)
#if ENABLE_THREAD_SAFETY

TYPED_TEST(FixedListTest, ThreadSafety) {
  std::thread t1([&]() {
    for (int i = 0; i < 5; ++i) {
      this->list.push_back(i + 10);
    }
  });

  std::thread t2([&]() {
    for (int i = 0; i < 5; ++i) {
      this->list.pop_front();
    }
  });

  t1.join();
  t2.join();

  EXPECT_LE(this->list.size(), this->list.capacity());
}
#endif
} // namespace ESTL
