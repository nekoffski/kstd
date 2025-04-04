#include "Core.hh"

#include "kstd/Functional.hh"

TEST(GuardCallTests, callOnDestructor) {
    bool called = false;
    {
        kstd::GuardCall guard{ [&]() { called = true; } };
        ASSERT_FALSE(called);
    }
    ASSERT_TRUE(called);
}
