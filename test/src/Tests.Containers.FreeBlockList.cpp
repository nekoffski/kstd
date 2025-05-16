#include "kstd/containers/FreeBlockList.hh"

#include "Core.hh"

using namespace kstd;

TEST(FreeBlockListTests, spaceLeft) {
    FreeBlockList fbl{ 1024u };
    ASSERT_EQ(fbl.spaceLeft(), 1024u);

    fbl.acquireBlock(100u);
    ASSERT_EQ(fbl.spaceLeft(), 924u);
}

TEST(FreeBlockListTests, acquireRelease) {
    FreeBlockList fbl{ 1024u };

    auto r = fbl.acquireBlock(512u);
    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r->offset, 0u);
    auto r2 = fbl.acquireBlock(512u);
    ASSERT_TRUE(r2.has_value());
    ASSERT_EQ(r2->offset, 512u);

    ASSERT_FALSE(fbl.acquireBlock(256u).has_value());

    fbl.releaseBlock(*r);

    auto r3 = fbl.acquireBlock(256u);
    ASSERT_TRUE(r3.has_value());
    ASSERT_EQ(r3->offset, 0u);
}

TEST(FreeBlockListTests, mergingBlocks) {
    FreeBlockList fbl{ 1024u };
    auto r1 = fbl.acquireBlock(512u);
    auto r2 = fbl.acquireBlock(256u);
    fbl.releaseBlock(*r1);
    fbl.releaseBlock(*r2);

    auto r3 = fbl.acquireBlock(900u);
    ASSERT_TRUE(r3.has_value());
    ASSERT_EQ(r3->offset, 0u);
}
