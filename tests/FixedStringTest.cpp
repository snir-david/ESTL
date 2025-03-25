//
// Created by SnirN on 3/25/2025.
//
#include <gtest/gtest.h>
#include "../FixedString.hpp"

namespace ESTL {
    template<typename StringType>
    class FixedStringTest : public ::testing::Test {
    protected:
        std::size_t bufferSize = 20;
        StringType str;

        FixedStringTest()
            : str(bufferSize) {}
    };

    template<std::size_t N>
    class FixedStringTest<CTString<N>> : public ::testing::Test {
    protected:
        static const std::size_t bufferSize = N;
        CTString<N> str;

        FixedStringTest() : str() {
        }
    };

    template<std::size_t N>
    const std::size_t FixedStringTest<CTString<N>>::bufferSize;

    using TestTypes = ::testing::Types<CTString<20>, RTString>;
    TYPED_TEST_SUITE(FixedStringTest, TestTypes);

    TYPED_TEST(FixedStringTest, Constructor) {
        EXPECT_EQ(this->str.size(), 0);
        EXPECT_EQ(this->str.capacity(), this->bufferSize);
    }

    TYPED_TEST(FixedStringTest, Append) {
        this->str.append("Hello");
        EXPECT_EQ(this->str.size(), 5);
        EXPECT_STREQ(this->str.c_str(), "Hello");
    }

    TYPED_TEST(FixedStringTest, Clear) {
        this->str.append("Hello");
        this->str.clear();
        EXPECT_EQ(this->str.size(), 0);
        EXPECT_STREQ(this->str.c_str(), "");
    }

    TYPED_TEST(FixedStringTest, Insert) {
        this->str.append("Hello");
        this->str.insert(5, " World");
        EXPECT_EQ(this->str.size(), 11);
        EXPECT_STREQ(this->str.c_str(), "Hello World");
    }

    TYPED_TEST(FixedStringTest, Erase) {
        this->str.append("Hello World");
        this->str.erase(5, 6);
        EXPECT_EQ(this->str.size(), 5);
        EXPECT_STREQ(this->str.c_str(), "Hello");
    }

    TYPED_TEST(FixedStringTest, Replace) {
        this->str.append("Hello World");
        this->str.replace(6, 5, "ESTL");
        EXPECT_EQ(this->str.size(), 10);
        EXPECT_STREQ(this->str.c_str(), "Hello ESTL");
    }

    TYPED_TEST(FixedStringTest, Find) {
        this->str.append("Hello World");
        EXPECT_EQ(this->str.find("World"), 6);
        EXPECT_EQ(this->str.find("ESTL"), std::string::npos);
    }

    TYPED_TEST(FixedStringTest, RFind) {
        this->str.append("Hello World World");
        EXPECT_EQ(this->str.rfind("World"), 12);
        EXPECT_EQ(this->str.rfind("ESTL"), std::string::npos);
    }

    TYPED_TEST(FixedStringTest, StartsWith) {
        this->str.append("Hello World");
        EXPECT_TRUE(this->str.starts_with("Hello"));
        EXPECT_FALSE(this->str.starts_with("World"));
    }

    TYPED_TEST(FixedStringTest, EndsWith) {
        this->str.append("Hello World");
        EXPECT_TRUE(this->str.ends_with("World"));
        EXPECT_FALSE(this->str.ends_with("Hello"));
    }

    TYPED_TEST(FixedStringTest, Overflow) {
        EXPECT_THROW(this->str.append("This is a very long string that exceeds the buffer size"), std::out_of_range);
    }

    TYPED_TEST(FixedStringTest, Underflow) {
        EXPECT_THROW(this->str.pop_back(), std::out_of_range);
    }

//    TYPED_TEST(FixedStringTest, Substr) {
//        this->str.append("Hello World");
//        auto substr = this->str.substr(6, 5);
//        EXPECT_EQ(substr.size(), 5);
//        EXPECT_STREQ(substr.c_str(), "World");
//    }

    TYPED_TEST(FixedStringTest, Iterator) {
        this->str.append("Hello");
        std::string result;
        for (auto it = this->str.begin(); it != this->str.end(); ++it) {
            result += *it;
        }
        EXPECT_EQ(result, "Hello");
    }
}