//
// Created by SnirN on 3/26/2025.
//
#include "../FixedMap/AVLTree.hpp"
#include "../FixedMap/BalancedTreeFactory.hpp"
#include "../FixedMap/RedBlackTree.hpp"
#include <gtest/gtest.h>

namespace ESTL {

    class BalancedTreeTest : public ::testing::TestWithParam<TreeType> {
    protected:
        const size_t DEFAULT_CAPACITY = 10;
        std::unique_ptr<BalancedTree<int, std::string>> tree;

        void SetUp() override {
            tree = BalancedTreeFactory<int, std::string>::createTree(GetParam(), DEFAULT_CAPACITY);
        }
    };


    TEST_P(BalancedTreeTest, Constructor) {
        EXPECT_EQ(tree->size(), 0);
        EXPECT_TRUE(tree->empty());
    }

    TEST_P(BalancedTreeTest, Insert) {
        EXPECT_TRUE(tree->insert(1, "one"));
        EXPECT_TRUE(tree->insert(2, "two"));
        EXPECT_EQ(tree->size(), 2);
        EXPECT_EQ(*tree->find(1), "one");
        EXPECT_EQ(*tree->find(2), "two");

        // Insert duplicate key
        EXPECT_FALSE(tree->insert(1, "one_duplicate"));
        EXPECT_EQ(*tree->find(1), "one");// Original value unchanged
    }

    TEST_P(BalancedTreeTest, Erase) {
        EXPECT_TRUE(tree->insert(1, "one"));
        EXPECT_TRUE(tree->insert(2, "two"));
        EXPECT_TRUE(tree->insert(3, "three"));
        EXPECT_EQ(tree->size(), 3);
        EXPECT_TRUE(tree->erase(2));
        EXPECT_EQ(tree->size(), 2);
        EXPECT_EQ(tree->find(2), nullptr);
        EXPECT_FALSE(tree->erase(2));// Erase non-existent key
        EXPECT_EQ(tree->size(), 2);
    }

    TEST_P(BalancedTreeTest, Find) {
        EXPECT_TRUE(tree->insert(1, "one"));
        EXPECT_TRUE(tree->insert(2, "two"));
        EXPECT_EQ(*tree->find(1), "one");
        EXPECT_EQ(*tree->find(2), "two");
        EXPECT_EQ(tree->find(3), nullptr);
    }

    TEST_P(BalancedTreeTest, Clear) {
        EXPECT_TRUE(tree->insert(1, "one"));
        EXPECT_TRUE(tree->insert(2, "two"));
        EXPECT_TRUE(tree->insert(3, "three"));
        EXPECT_EQ(tree->size(), 3);
        tree->clear();
        EXPECT_EQ(tree->size(), 0);
        EXPECT_TRUE(tree->empty());
        EXPECT_EQ(tree->find(1), nullptr);
        EXPECT_EQ(tree->find(2), nullptr);
        EXPECT_EQ(tree->find(3), nullptr);
    }

    TEST_P(BalancedTreeTest, Iterator) {
        EXPECT_TRUE(tree->insert(1, "one"));
        EXPECT_TRUE(tree->insert(2, "two"));
        EXPECT_TRUE(tree->insert(3, "three"));

        std::vector<std::pair<int, std::string>> result;
        for (auto it = tree->minimum(); it != nullptr; it = tree->next(it)) {
            result.emplace_back(it->key, it->value);
        }

        EXPECT_EQ(result.size(), 3);
        EXPECT_EQ(result[0].first, 1);
        EXPECT_EQ(result[0].second, "one");
        EXPECT_EQ(result[1].first, 2);
        EXPECT_EQ(result[1].second, "two");
        EXPECT_EQ(result[2].first, 3);
        EXPECT_EQ(result[2].second, "three");
    }

    TEST_P(BalancedTreeTest, Overflow) {
        for (int i = 0; i < this->DEFAULT_CAPACITY; ++i) {
            EXPECT_TRUE(tree->insert(i, "value" + std::to_string(i)));
        }
        EXPECT_EQ(tree->size(), this->DEFAULT_CAPACITY);

        // Attempt to exceed capacity
        EXPECT_FALSE(tree->insert(this->DEFAULT_CAPACITY, "overflow"));
        EXPECT_EQ(tree->size(), this->DEFAULT_CAPACITY);
    }

    TEST_P(BalancedTreeTest, Underflow) {
        EXPECT_FALSE(tree->erase(1));// Empty tree
        EXPECT_EQ(tree->size(), 0);

        tree->insert(1, "one");
        EXPECT_TRUE(tree->erase(1));
        EXPECT_FALSE(tree->erase(1));// Now empty again
        EXPECT_EQ(tree->size(), 0);
    }

    INSTANTIATE_TEST_SUITE_P(TreeTypes, BalancedTreeTest, ::testing::Values(TreeType::RedBlack, TreeType::AVL));


}// namespace ESTL