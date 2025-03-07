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
    # 强制优先使用当前目录的 mingw64/bin/gcc
    export PATH="$PWD/mingw64/bin:$PATH"
    
    # Windows 下的编译命令（静态链接）
    gcc -Wall -Wextra \
        $INCLUDE_DIRS \
        -I"mingw64/include" \
        -I"mingw64/include/libxml2" \
        -o build/arXmlTool.exe $SRC_FILES \
        -L"mingw64/lib" \
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

# Test counters
TOTAL_TESTS=0
PASSED_TESTS=0

# 在文件开头添加函数定义
run_command() {
    echo "Command: $@"
    "$@"
    echo ""
}

rm -rf testbench/results
echo "开始测试..."

echo "-------------------"
echo "Test Case 1: Merge Tests"
echo "-------------------"

echo "Test Case 1.1: Basic Merge with Duplicates"
run_command ./build/arXmlTool.exe merge \
    -a testbench/cases/1.1/duplicate1.arxml \
    -a testbench/cases/1.1/duplicate2.arxml \
    -m testbench/results/1.1/merged_duplicate.arxml

echo "Test Case 1.2: Merge Multiple Duplicates"
run_command ./build/arXmlTool.exe merge \
    -a testbench/cases/1.2/multiple1.arxml \
    -a testbench/cases/1.2/multiple2.arxml \
    -a testbench/cases/1.2/multiple3.arxml \
    -m testbench/results/1.2/merged_multiple.arxml

echo "Test Case 1.3: Merge Different Structures"
run_command ./build/arXmlTool.exe merge \
    -a testbench/cases/1.3/structure1.arxml \
    -a testbench/cases/1.3/structure2.arxml \
    -m testbench/results/1.3/merged_structure.arxml

echo "Test Case 1.4: Merge Non-conflicting Containers"
run_command ./build/arXmlTool.exe merge \
    -a testbench/cases/1.4/noconflict1.arxml \
    -a testbench/cases/1.4/noconflict2.arxml \
    -a testbench/cases/1.4/noconflict3.arxml \
    -m testbench/results/1.4/merged_noconflict.arxml

echo "Test Case 1.5: Merge Non-conflicting Containers with Tag Sorting (Ascending)"
run_command ./build/arXmlTool.exe merge \
    -a testbench/cases/1.5/noconflict1.arxml \
    -a testbench/cases/1.5/noconflict2.arxml \
    -a testbench/cases/1.5/noconflict3.arxml \
    -s asc -t AR-PACKAGES \
    -m testbench/results/1.5/merged_noconflict.arxml

echo "Test Case 1.6: Merge Non-conflicting Containers with Tag Sorting (Descending)"
run_command ./build/arXmlTool.exe merge \
    -a testbench/cases/1.6/noconflict1.arxml \
    -a testbench/cases/1.6/noconflict2.arxml \
    -a testbench/cases/1.6/noconflict3.arxml \
    -s desc -t AR-PACKAGES \
    -m testbench/results/1.6/merged_noconflict.arxml

echo "-------------------"
echo "Test Case 2: Command File Tests"
echo "-------------------"

echo "Test Case 2.1: Command File Input"
run_command ./build/arXmlTool.exe merge -f testbench/cases/2.1/command.txt

echo "Test Case 2.2: Command File With Multiple Lines Input"
run_command ./build/arXmlTool.exe merge -f testbench/cases/2.2/command.txt

echo "Test Case 2.3: Command File With Multiple Lines Input in Ascending Order with Tag"
run_command ./build/arXmlTool.exe merge -f testbench/cases/2.3/command.txt

echo "-------------------"
echo "Test Case 3: Indentation Tests"
echo "-------------------"

echo "Test Case 3.1: Tab Indentation"
run_command ./build/arXmlTool.exe merge \
    -a testbench/cases/3.1/indent_test.arxml \
    -m testbench/results/3.1/merged_tab.arxml -i tab

echo "Test Case 3.2: 2-Space Indentation"
run_command ./build/arXmlTool.exe merge \
    -a testbench/cases/3.2/indent_test.arxml \
    -m testbench/results/3.2/merged_2space.arxml -i 2

echo "Test Case 3.3: 4-Space Indentation (Default)"
run_command ./build/arXmlTool.exe merge \
    -a testbench/cases/3.3/indent_test.arxml \
    -m testbench/results/3.3/merged_4space.arxml -i 4

echo "-------------------"
echo "Test Case 4: Format Mode Tests"
echo "-------------------"

echo "Test Case 4.1: Format with Tab Indentation (In-place)"
mkdir -p testbench/results/4.1
cp testbench/cases/4.1/format_test.arxml testbench/results/4.1/format_test_tab.arxml
run_command ./build/arXmlTool.exe format \
    -a testbench/results/4.1/format_test_tab.arxml -i tab

echo "Test Case 4.2: Format with 2-Space Indentation (Output Directory)"
run_command ./build/arXmlTool.exe format \
    -a testbench/cases/4.2/format_test.arxml -i 2 -o testbench/results/4.2

echo "Test Case 4.3: Format Multiple Files"
run_command ./build/arXmlTool.exe format \
    -a testbench/cases/4.3/format_test.arxml \
    -a testbench/cases/4.3/indent_test.arxml \
    -i 4 -o testbench/results/4.3

echo "Test Case 4.4: Format with Sorting"
run_command ./build/arXmlTool.exe format \
    -a testbench/cases/4.4/format_test.arxml \
    -s asc -o testbench/results/4.4

echo "Test Case 4.5: Format with Sorting and Indentation"
run_command ./build/arXmlTool.exe format \
    -a testbench/cases/4.5/format_test.arxml \
    -s desc -i 2 -o testbench/results/4.5

echo "Test Case 4.6: Format with Sorting and Indentation"
run_command ./build/arXmlTool.exe format \
    -a testbench/cases/4.6/format_test.arxml \
    -s desc -i tab -o testbench/results/4.6

echo "Test Case 4.7: Format with any Indentation"
run_command ./build/arXmlTool.exe format \
    -a testbench/cases/4.7/format_test.arxml \
    -i 8 -o testbench/results/4.7

echo "Test Case 4.8: Format with any Indentation in Ascending Order with Tag"
run_command ./build/arXmlTool.exe format \
    -a testbench/cases/4.8/format_test.arxml \
    -i 2 -s asc -t AR-PACKAGES -o testbench/results/4.8

echo "-------------------"
echo "Test Case 5: Output Directory Tests"
echo "-------------------"

echo "Test Case 5.1: Output Directory Creation"
mkdir -p testbench/results/5.1
# Copy executable to test directory | 复制可执行文件到测试目录
cp build/arXmlTool.exe testbench/results/5.1/
(cd testbench/results/5.1 && run_command ./arXmlTool.exe merge \
    -a ../../cases/5.1/noconflict1.arxml \
    -a ../../cases/5.1/noconflict2.arxml \
    -m output.arxml)
rm testbench/results/5.1/arXmlTool.exe

echo "Test Case 5.2: Output Directory Creation To Specific Directory"
run_command ./build/arXmlTool.exe merge \
    -a testbench/cases/5.2/noconflict1.arxml \
    -a testbench/cases/5.2/noconflict2.arxml \
    -m output.arxml -o testbench/results/5.2
