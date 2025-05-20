#include "Core.hh"

#include <vector>

#include "kstd/Algorithms.hh"
#include "kstd/containers/SlotBuffer.hh"

using namespace kstd;

TEST(FilterTests, vector) {
    std::vector<int> values = { 1, 2, 3, 4 };
    auto filtered           = filter(values, [](int x) { return x % 2 == 0; });
    ASSERT_EQ(filtered.size(), 2);
    ASSERT_EQ(filtered[0], 2);
    ASSERT_EQ(filtered[1], 4);

    filtered =
      filter(std::vector<int>{ 1, 2, 3, 4 }, [](int x) { return x % 2 == 0; });
    ASSERT_EQ(filtered.size(), 2);
    ASSERT_EQ(filtered[0], 2);
    ASSERT_EQ(filtered[1], 4);
}

TEST(TransformTests, vector) {
    std::vector<int> values = { 1, 2, 3, 4 };
    auto out = transform(values, [&](const auto& x) -> float { return x * 2.0f; });
    for (int i = 0; i < values.size(); ++i) ASSERT_EQ(out[i], values[i] * 2.0f);

    const std::vector<int> cvalues = { 1, 2, 3, 4 };
    auto cout = transform(values, [&](const auto& x) -> float { return x * 2.0f; });
    for (int i = 0; i < values.size(); ++i) ASSERT_EQ(cout[i], cvalues[i] * 2.0f);
}

TEST(TransformTests, slotBuffer) {
    kstd::StackSlotBuffer<int, 16> values;
    values.insert(1);
    values.insert(2);
    values.insert(3);
    values.insert(4);
    auto out = transform(values, [&](const auto& x) { return x * 2.0f; });
    int i    = 0;
    for (auto& v : values) ASSERT_EQ(2.0f * v, out[i++]);
}
