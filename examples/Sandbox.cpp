
#include "kstd/Core.hh"
#include "kstd/Log.hh"

int main() {
    kstd::log::init("sandbox");
    kstd::log::info("Hello world!");

    return 0;
}
