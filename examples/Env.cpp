

#include <kstd/Env.hh>
#include <kstd/Log.hh>

int main() {
    kstd::log::init("Env");

    auto v = kstd::getEnv<kstd::f32>("TEST_ENV").value_or(0);
    kstd::log::debug("v={}", v);

    return 0;
}