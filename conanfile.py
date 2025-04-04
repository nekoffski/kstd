import os
from pathlib import Path

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


requirements = [
    "fmt/10.2.1",
    "spdlog/1.14.1",
    "gtest/1.15.0",
    "benchmark/1.9.0",
    "nlohmann_json/3.11.3",
]


class Recipe(ConanFile):
    name = "kstd"
    version = "1.0"
    license = "MIT"
    author = "Kacper Czyz"

    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": False}

    exports_sources = "CMakeLists.txt", "src/*", "examples/*", "test/*", "benchmark/*"

    def requirements(self):
        for requirement in requirements:
            self.requires(requirement, transitive_headers=True)

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.variables["KSTD_ENABLE_COVERAGE"] = int(
            os.getenv('KSTD_ENABLE_COVERAGE', 0))
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

        self.test()

        if int(os.getenv("KSTD_RUN_BENCHMARKS", 0)):
            self.benchmark()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def test(self):
        self.output.info("Running tests...")
        self.run('ctest -VV --output-on-failure')

    def benchmark(self):
        self.output.info("Running benchmarks...")
        self.run("benchmark/kstd.benchmark")

    def package_info(self):
        self.cpp_info.libs = ["kstd"]
        self.cpp_info.requires = [
            'fmt::fmt',
            'spdlog::spdlog',
            'gtest::gtest',
            'benchmark::benchmark_main',
            'nlohmann_json::nlohmann_json'
        ]
