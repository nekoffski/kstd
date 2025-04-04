#!/bin/bash

SRC=$(dirname "$0")/../
rm ${SRC}/build -rf
conan build . --profile ${SRC}/conf/profiles/debug
