#include <gtest/gtest.h>

#include <memory>

#include "kstd/RTTI.hh"

namespace {

struct Base : kstd::RTTI {
    virtual int foo() = 0;
};

struct Derived : Base {
    std::type_index getType() const override { return typeid(Derived); }
    int foo() override { return 1; }
};

}  // namespace

TEST(RTTITests, hierarchy) {
    std::unique_ptr<Base> ptr = std::make_unique<Derived>();
    ASSERT_TRUE(ptr->is<Derived>());
}
