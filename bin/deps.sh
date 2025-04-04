#!/bin/bash

SRC=$(dirname "$0")/../
conan install ${SRC} --output-folder=${SRC} --build=missing --profile ${SRC}/conf/profiles/debug
