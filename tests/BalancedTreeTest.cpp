//
// Created by SnirN on 3/26/2025.
//
#include "../FixedMap/BalancedTreeFactory.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <string>

namespace ESTL {

    class BalancedTreeTest : public ::testing::TestWithParam<TreeType> {
    protected:
        const size_t DEFAULT_CAPACITY = 5000;
        std::unique_ptr<BalancedTree<int, std::string>> tree;
        TreeNode<int, std::string> *buffer;

        void SetUp() override {
            buffer = new TreeNode<int, std::string>[DEFAULT_CAPACITY];
            tree = BalancedTreeFactory<int, std::string>::createTree(GetParam
                                                                     (), buffer, DEFAULT_CAPACITY);
        }

        void TearDown() override {
          delete [] buffer;
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

    TEST_P(BalancedTreeTest, MultiThreadedInsertErase) {
        const int numThreads = 5;
        const int numOperations = 1000;

        auto insertTask = [this](int start, int end) {
            for (int i = start; i < end; ++i) {
              printf("insertTask %d\n", i);
                tree->insert(i, "value" + std::to_string(i));
            }
        };

        auto eraseTask = [this](int start, int end) {
            for (int i = start; i < end; ++i) {
              printf("eraseTask %d\n", i);
              tree->erase(i);
            }
        };


        this->tree->clear();

        std::vector<std::thread> threads;

        // Launch threads to perform insertions
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back(insertTask, i * numOperations, (i + 1) * numOperations);
        }

        // Wait for all insertion threads to finish
        for (auto &thread : threads) {
            thread.join();
        }

        EXPECT_EQ(tree->size(), numThreads * numOperations);

        threads.clear();

        // Launch threads to perform deletions
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back(eraseTask, i * numOperations, (i + 1) * numOperations);
        }

        // Wait for all deletion threads to finish
        for (auto &thread : threads) {
            thread.join();
        }

        EXPECT_EQ(tree->size(), 0);
    }

    INSTANTIATE_TEST_SUITE_P(TreeTypes, BalancedTreeTest, ::testing::Values
                             (TreeType::RedBlack, TreeType::AVL));


}// namespace ESTL