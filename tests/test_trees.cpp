#include <gtest/gtest.h>

#include <array>
#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <vector>

#include "avl/AVLTree.hpp"
#include "rbt/RedBlackTree.hpp"

namespace {

sdn::PacketRule makeRule(uint32_t id, uint8_t priority = 1) {
    return sdn::PacketRule{
        id,
        0x0A000000u + id,
        0xC0A80000u + id,
        priority,
    };
}

template <typename Tree, typename Validator>
void insertIncreasingAndValidate(Tree& tree, Validator validate) {
    for (uint32_t id = 1; id <= 50; ++id) {
        tree.insert(makeRule(id, static_cast<uint8_t>(id % 256)));
        EXPECT_NO_THROW(validate(tree));
        EXPECT_EQ(tree.size(), id);
        ASSERT_NE(tree.search(id), nullptr);
    }
}

template <typename Tree>
void expectPresentExcept(const Tree& tree,
                         const std::vector<uint32_t>& allIds,
                         const std::vector<uint32_t>& removedIds) {
    for (uint32_t id : allIds) {
        const bool removed = std::find(removedIds.begin(), removedIds.end(), id) != removedIds.end();
        if (removed) {
            EXPECT_EQ(tree.search(id), nullptr);
        } else {
            const sdn::PacketRule* rule = tree.search(id);
            ASSERT_NE(rule, nullptr);
            EXPECT_EQ(rule->id, id);
        }
    }
}

template <typename Tree, typename Validator>
void removeSequenceAndValidate(Tree& tree,
                               Validator validate,
                               const std::vector<uint32_t>& allIds,
                               const std::vector<uint32_t>& removeIds) {
    for (uint32_t id : allIds) {
        tree.insert(makeRule(id));
    }

    std::vector<uint32_t> removedIds;
    removedIds.reserve(removeIds.size());
    std::size_t expectedSize = allIds.size();

    for (uint32_t id : removeIds) {
        SCOPED_TRACE(id);
        EXPECT_TRUE(tree.remove(id));
        removedIds.push_back(id);
        --expectedSize;

        EXPECT_EQ(tree.size(), expectedSize);
        EXPECT_NO_THROW(validate(tree));
        expectPresentExcept(tree, allIds, removedIds);
    }

    EXPECT_FALSE(tree.remove(999));
    EXPECT_EQ(tree.size(), expectedSize);
    EXPECT_NO_THROW(validate(tree));
}

} // namespace

TEST(AVLTreeQA, ValidatesAfterEachIncreasingInsert) {
    sdn::AVLTree tree(50);

    insertIncreasingAndValidate(tree, sdn::validateAVL);

    EXPECT_TRUE(tree.validate());
    EXPECT_EQ(tree.size(), 50u);
    EXPECT_GT(tree.height(), 0);
}

TEST(AVLTreeQA, ValidatesAfterShuffledInsertsAndSearches) {
    sdn::AVLTree tree(16);
    const std::array<uint32_t, 15> ids = {
        20, 4, 26, 3, 9, 15, 30, 2, 7, 11, 18, 25, 28, 35, 1,
    };

    for (uint32_t id : ids) {
        tree.insert(makeRule(id));
        EXPECT_NO_THROW(sdn::validateAVL(tree));
    }

    EXPECT_EQ(tree.size(), ids.size());
    EXPECT_GT(tree.height(), 0);
    for (uint32_t id : ids) {
        const sdn::PacketRule* rule = tree.search(id);
        ASSERT_NE(rule, nullptr);
        EXPECT_EQ(rule->id, id);
    }
    EXPECT_EQ(tree.search(999), nullptr);
}

TEST(AVLTreeQA, UpsertKeepsSizeAndInvariants) {
    sdn::AVLTree tree;

    tree.insert(makeRule(10, 1));
    tree.insert(makeRule(10, 255));

    EXPECT_NO_THROW(sdn::validateAVL(tree));
    EXPECT_EQ(tree.size(), 1u);
    ASSERT_NE(tree.search(10), nullptr);
    EXPECT_EQ(tree.search(10)->priority, 255);
}

TEST(AVLTreeQA, ValidatesAfterBasicRemovals) {
    sdn::AVLTree tree(32);
    for (uint32_t id = 1; id <= 32; ++id) {
        tree.insert(makeRule(id));
    }

    for (uint32_t id : {1u, 16u, 32u, 8u, 24u}) {
        EXPECT_TRUE(tree.remove(id));
        EXPECT_NO_THROW(sdn::validateAVL(tree));
        EXPECT_EQ(tree.search(id), nullptr);
    }
    EXPECT_FALSE(tree.remove(999));
    EXPECT_NO_THROW(sdn::validateAVL(tree));
}

TEST(AVLTreeQA, RemovesRootAndKeepsRemainingNodesSearchable) {
    sdn::AVLTree tree;
    const std::vector<uint32_t> ids = {10, 5, 15, 3, 7, 12, 18};

    removeSequenceAndValidate(tree, sdn::validateAVL, ids, {10});
}

TEST(AVLTreeQA, ValidatesAfterLargerShuffledRemovalSequence) {
    sdn::AVLTree tree(64);
    std::vector<uint32_t> ids;
    ids.reserve(64);
    for (uint32_t id = 1; id <= 64; ++id) {
        ids.push_back(id);
    }

    const std::vector<uint32_t> removalOrder = {
        32, 1, 64, 16, 48, 8, 24, 40, 56, 4, 12, 20, 28, 36, 44, 52, 60,
        2, 6, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62,
        3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31,
        33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63,
    };

    removeSequenceAndValidate(tree, sdn::validateAVL, ids, removalOrder);
}

TEST(RedBlackTreeQA, ValidatesAfterEachIncreasingInsert) {
    sdn::RedBlackTree tree(50);

    insertIncreasingAndValidate(tree, sdn::validateRBT);

    EXPECT_TRUE(tree.validate());
    EXPECT_EQ(tree.size(), 50u);
    EXPECT_GT(tree.height(), 0);
    EXPECT_GT(tree.black_height(), 0);
}

TEST(RedBlackTreeQA, ValidatesAfterShuffledInsertsAndSearches) {
    sdn::RedBlackTree tree(16);
    const std::array<uint32_t, 15> ids = {
        20, 4, 26, 3, 9, 15, 30, 2, 7, 11, 18, 25, 28, 35, 1,
    };

    for (uint32_t id : ids) {
        tree.insert(makeRule(id));
        EXPECT_NO_THROW(sdn::validateRBT(tree));
    }

    EXPECT_EQ(tree.size(), ids.size());
    EXPECT_GT(tree.height(), 0);
    EXPECT_GT(tree.black_height(), 0);
    for (uint32_t id : ids) {
        const sdn::PacketRule* rule = tree.search(id);
        ASSERT_NE(rule, nullptr);
        EXPECT_EQ(rule->id, id);
    }
    EXPECT_EQ(tree.search(999), nullptr);
}

TEST(RedBlackTreeQA, UpsertKeepsSizeAndInvariants) {
    sdn::RedBlackTree tree;

    tree.insert(makeRule(10, 1));
    tree.insert(makeRule(10, 255));

    EXPECT_NO_THROW(sdn::validateRBT(tree));
    EXPECT_EQ(tree.size(), 1u);
    ASSERT_NE(tree.search(10), nullptr);
    EXPECT_EQ(tree.search(10)->priority, 255);
}

TEST(RedBlackTreeQA, ValidatesAfterBasicRemovals) {
    sdn::RedBlackTree tree(32);
    for (uint32_t id = 1; id <= 32; ++id) {
        tree.insert(makeRule(id));
    }

    for (uint32_t id : {1u, 16u, 32u, 8u, 24u}) {
        EXPECT_TRUE(tree.remove(id));
        EXPECT_NO_THROW(sdn::validateRBT(tree));
        EXPECT_EQ(tree.search(id), nullptr);
    }
    EXPECT_FALSE(tree.remove(999));
    EXPECT_NO_THROW(sdn::validateRBT(tree));
}

TEST(RedBlackTreeQA, RemovesRootAndKeepsRemainingNodesSearchable) {
    sdn::RedBlackTree tree;
    const std::vector<uint32_t> ids = {10, 5, 15, 3, 7, 12, 18};

    removeSequenceAndValidate(tree, sdn::validateRBT, ids, {10});
}

TEST(RedBlackTreeQA, ValidatesAfterLargerShuffledRemovalSequence) {
    sdn::RedBlackTree tree(64);
    std::vector<uint32_t> ids;
    ids.reserve(64);
    for (uint32_t id = 1; id <= 64; ++id) {
        ids.push_back(id);
    }

    const std::vector<uint32_t> removalOrder = {
        32, 1, 64, 16, 48, 8, 24, 40, 56, 4, 12, 20, 28, 36, 44, 52, 60,
        2, 6, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62,
        3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31,
        33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63,
    };

    removeSequenceAndValidate(tree, sdn::validateRBT, ids, removalOrder);
}
