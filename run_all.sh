#!/bin/bash

# Run compilation script
echo "Step 1: Compile program"
if [ "$OS" = "Windows_NT" ]; then
    # Windows environment (assuming MinGW)
    XML2_PATH="C:/msys64/mingw64"
    gcc -Wall -Wextra \
        -I"$XML2_PATH/include/libxml2" \
        -o arXmlTool.exe arXmlTool.c \
        -static \
        -L"$XML2_PATH/lib" \
        -lxml2 -liconv -lz -llzma -lws2_32
else
    # Linux/Unix environment
    CFLAGS=$(pkg-config --cflags libxml-2.0)
    LIBS=$(pkg-config --libs libxml-2.0)
    gcc -Wall -Wextra $CFLAGS -o arXmlTool.exe arXmlTool.c $LIBS
fi
if [ $? -ne 0 ]; then
    echo "Compilation failed, test aborted"
    exit 1
fi

# Prepare test environment
echo "Step 2: Prepare test environment"
mkdir -p test_files
if [ $? -ne 0 ]; then
    echo "Test environment preparation failed"
    exit 1
fi

# Run tests
echo "Step 3: Run tests"
./run_tests.sh 