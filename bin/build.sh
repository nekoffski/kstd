#!/bin/bash

SRC=$(dirname "$0")/../

conan install ${SRC}/conan/ --output-folder=${SRC}/build --build=missing --profile ${SRC}/conan/profiles/debug
cd ${SRC}/build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug -DKSTD_ENABLE_UNIT_TESTS=On \
    -DKSTD_ENABLE_COVERAGE=Off -DKSTD_ENABLE_BENCHMARKS=Off
cmake --build . -j`nproc`