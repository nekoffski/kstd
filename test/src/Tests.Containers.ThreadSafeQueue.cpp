#include "kstd/containers/ThreadSafeQueue.hh"

#include <barrier>
#include <thread>

#include "Core.hh"

using namespace kstd;

TEST(FixedSizeThreadSafeQueueTests, basic) {
    FixedSizeThreadSafeQueue<int, 2> q;
    ASSERT_TRUE(q.tryPush(1));
    ASSERT_TRUE(q.tryPush(2));
    ASSERT_FALSE(q.tryPush(3));
    ASSERT_EQ(q.pop(), 1);
    ASSERT_TRUE(q.tryPush(3));
}
