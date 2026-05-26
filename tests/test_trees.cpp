#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <initializer_list>

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
