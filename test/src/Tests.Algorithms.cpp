#include "Core.hh"

#include <vector>

#include "kstd/Algorithms.hh"
#include "kstd/containers/SlotBuffer.hh"

using namespace kstd;

TEST(TransformTests, vector) {
    std::vector<int> values = { 1, 2, 3, 4 };
    auto out = transform(values, [&](auto& x) -> float { return x * 2.0f; });
    for (int i = 0; i < values.size(); ++i) ASSERT_EQ(out[i], values[i] * 2.0f);

    const std::vector<int> cvalues = { 1, 2, 3, 4 };
    auto cout = transform(values, [&](const auto& x) -> float { return x * 2.0f; });
    for (int i = 0; i < values.size(); ++i) ASSERT_EQ(cout[i], cvalues[i] * 2.0f);
}

TEST(TransformTests, slotBuffer) {
    kstd::SlotBuffer<int, 16> values;
    values.insert(1);
    values.insert(2);
    values.insert(3);
    values.insert(4);
    auto out = transform(values, [&](const auto& x) { return x * 2.0f; });
    int i    = 0;
    for (auto& v : values) ASSERT_EQ(2.0f * v, out[i++]);
}
