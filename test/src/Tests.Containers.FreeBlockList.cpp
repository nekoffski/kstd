#include "kstd/containers/FreeBlockList.hh"

#include "Core.hh"

using namespace kstd;

TEST(FreeBlockListTests, spaceLeft) {
    FreeBlockList fbl{ 1024u };
    ASSERT_EQ(fbl.spaceLeft(), 1024u);

    fbl.acquireBlock(64u);
    ASSERT_EQ(fbl.spaceLeft(), 1024u - 64u);
}

TEST(FreeBlockListTests, acquireRelease) {
    FreeBlockList fbl{ 2048u };

    auto r = fbl.acquireBlock(512u);
    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r->offset, 0u);
    ASSERT_EQ(r->size, 512u);

    auto r2 = fbl.acquireBlock(512u);
    ASSERT_TRUE(r2.has_value());
    ASSERT_EQ(r2->offset, 512u);
    ASSERT_EQ(r2->size, 512u);

    ASSERT_FALSE(fbl.acquireBlock(2048u).has_value());

    fbl.releaseBlock(*r);

    auto r3 = fbl.acquireBlock(256u);
    ASSERT_TRUE(r3.has_value());
    ASSERT_EQ(r3->offset, 0u);
    ASSERT_EQ(r3->size, 256u);
}

TEST(FreeBlockListTests, mergingBlocks) {
    FreeBlockList fbl{ 1024u };
    auto r1 = fbl.acquireBlock(512u);
    ASSERT_EQ(r1->offset, 0u);
    ASSERT_EQ(r1->size, 512u);

    auto r2 = fbl.acquireBlock(256u);
    ASSERT_EQ(r2->offset, 512u);
    ASSERT_EQ(r2->size, 256u);

    fbl.releaseBlock(*r1);
    fbl.releaseBlock(*r2);

    auto r3 = fbl.acquireBlock(128u * 7);
    ASSERT_TRUE(r3.has_value());
    ASSERT_EQ(r3->offset, 0u);
    ASSERT_EQ(r3->size, 128u * 7);
}
