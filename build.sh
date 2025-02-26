#!/bin/bash

# Set script encoding to UTF-8
export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8

# Color definitions (may not be supported in Windows)
if [ "$OS" = "Windows_NT" ]; then
    GREEN=""
    RED=""
    NC=""
else
    GREEN='\033[0;32m'
    RED='\033[0;31m'
    NC='\033[0m'
fi

# Create build directory
mkdir -p build

# 1. Clean and compile
rm -f build/arXmlTool.exe
echo "开始编译 arXmlTool..."

# Source files
SRC_FILES="src/main/arxml_tool.c \
          src/main/options.c \
          src/command/command.c \
          src/operations/merge.c \
          src/operations/format.c \
          src/utils/fs_utils.c \
          src/utils/xml_utils.c"

# Include directories
INCLUDE_DIRS="-Isrc/main -Isrc/command -Isrc/operations -Isrc/utils"

# 检测操作系统
if [ "$OS" = "Windows_NT" ]; then
    # Windows 环境（假设使用 MinGW）
    XML2_PATH="env/mingw64"
    GCC_PATH="env/mingw64/bin/gcc.exe"
    
    # Windows 下的编译命令（静态链接）
    $GCC_PATH -Wall -Wextra \
        $INCLUDE_DIRS \
        -I"$XML2_PATH/include/libxml2" \
        -o build/arXmlTool.exe $SRC_FILES \
        -L"$XML2_PATH/lib" \
        -static \
        -lxml2 -lz -llzma -liconv -lws2_32 \
        -DLIBXML_STATIC
else
    # Linux/Unix 环境
    CFLAGS=$(pkg-config --cflags libxml-2.0)
    LIBS=$(pkg-config --libs libxml-2.0)
    gcc -Wall -Wextra $INCLUDE_DIRS $CFLAGS -o build/arXmlTool.exe $SRC_FILES $LIBS
fi

# 检查编译结果
if [ $? -eq 0 ]; then
    echo "编译成功！"
    echo "生成可执行文件: build/arXmlTool.exe"
else
    echo "编译失败！"
    exit 1
fi