#!/bin/bash

SRC=$(dirname "$0")/../
yes | conan remove kstd/*
conan create ${SRC} --build=missing --profile ${SRC}/conf/profiles/debug
