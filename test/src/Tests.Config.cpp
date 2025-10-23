#include "Core.hh"

#include "kstd/Config.hh"

using namespace kstd;

struct ConfigTests : ::testing::Test {
    ConfigTests() : cfg(Config{}) {}

    ConfigStorage cfg;
};

TEST_F(ConfigTests, keyNotFound) {
    EXPECT_THROW({ readConf<u32>("notexistingkey"); }, kstd::Error);
}

TEST_F(ConfigTests, valid) {
    setConf("a", 13);
    setConf("b", 13.37f);
    setConf("c", "abcd");

    EXPECT_EQ(readConf<u32>("a"), 13);
    EXPECT_EQ(readConf<u32>("b"), 13);
    EXPECT_FLOAT_EQ(readConf<f32>("b"), 13.37);
    EXPECT_EQ(readConf("c"), std::string{ "abcd" });
}

TEST_F(ConfigTests, wrongType) {
    setConf("a", "abcd");

    EXPECT_THROW({ readConf<u32>("a"); }, kstd::Error);
}
