#!/bin/bash

SRC=$(dirname "$0")/../
conan build . --profile ${SRC}/conf/profiles/debug
