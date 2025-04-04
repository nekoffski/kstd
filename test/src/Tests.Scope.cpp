#include <gtest/gtest.h>

#include "kstd/Scope.hh"

struct ScopeTests : testing::Test {
    int calls = 0;
};

TEST_F(ScopeTests, scopeExit) {
    {
        ON_SCOPE_EXIT { calls++; };
        EXPECT_EQ(calls, 0);
    }
    EXPECT_EQ(calls, 1);

    try {
        ON_SCOPE_EXIT { calls++; };
        EXPECT_EQ(calls, 1);

        throw 1;
    } catch (...) {
    }

    EXPECT_EQ(calls, 2);
}

TEST_F(ScopeTests, scopeFail) {
    {
        ON_SCOPE_FAIL { calls++; };
    }
    EXPECT_EQ(calls, 0);

    try {
        ON_SCOPE_FAIL { calls++; };
        EXPECT_EQ(calls, 0);

        throw 1;
    } catch (...) {
    }

    EXPECT_EQ(calls, 1);
}

TEST_F(ScopeTests, scopeSuccess) {
    try {
        ON_SCOPE_SUCCESS { calls++; };

        throw 1;
    } catch (...) {
    }

    EXPECT_EQ(calls, 0);

    {
        ON_SCOPE_SUCCESS { calls++; };
        EXPECT_EQ(calls, 0);
    }
    EXPECT_EQ(calls, 1);
}
