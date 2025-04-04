#!/bin/bash


KSTD_ENABLE_COVERAGE=1 KSTD_RUN_BENCHMARKS=0 ./bin/build.sh

cd build
source ../venv/bin/activate

rm -rf ../coverage && mkdir ../coverage
gcovr -f ../src/ -r . --exclude-throw-branches --xml --output ../coverage/coverage.xml
