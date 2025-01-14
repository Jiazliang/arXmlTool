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

# 1. Clean and compile
rm -f arXmlTool.exe
echo "开始编译 arXmlTool..."

# 检测操作系统
if [ "$OS" = "Windows_NT" ]; then
    # Windows 环境（假设使用 MinGW）
    XML2_PATH="C:/msys64/mingw64"
    
    # Windows 下的编译命令（静态链接）
    gcc -Wall -Wextra \
        -I"$XML2_PATH/include/libxml2" \
        -o arXmlTool.exe arXmlTool.c \
        -static \
        -L"$XML2_PATH/lib" \
        -lxml2 -liconv -lz -llzma -lws2_32
else
    # Linux/Unix 环境
    CFLAGS=$(pkg-config --cflags libxml-2.0)
    LIBS=$(pkg-config --libs libxml-2.0)
    gcc -Wall -Wextra $CFLAGS -o arXmlTool.exe arXmlTool.c $LIBS
fi

# 检查编译结果
if [ $? -eq 0 ]; then
    echo "编译成功！"
    echo "生成可执行文件: arXmlTool.exe"
else
    echo "编译失败！"
    exit 1
fi 

# Test counters
TOTAL_TESTS=0
PASSED_TESTS=0

echo "开始测试..."

echo "-------------------"
echo "Test Case 1: Merge Tests"
echo "-------------------"

echo "Test Case 1.1: Basic Merge with Duplicates"
rm -rf testbench/results/1.1/*
echo "Command: ./arXmlTool.exe merge -a testbench/cases/1.1/duplicate1.arxml -a testbench/cases/1.1/duplicate2.arxml -m testbench/results/1.1/merged_duplicate.arxml"
./arXmlTool.exe merge -a testbench/cases/1.1/duplicate1.arxml -a testbench/cases/1.1/duplicate2.arxml -m testbench/results/1.1/merged_duplicate.arxml
echo ""

echo "Test Case 1.2: Merge Multiple Duplicates"
rm -rf testbench/results/1.2/*
echo "Command: ./arXmlTool.exe merge -a testbench/cases/1.2/multiple1.arxml -a testbench/cases/1.2/multiple2.arxml -a testbench/cases/1.2/multiple3.arxml -m testbench/results/1.2/merged_multiple.arxml"
./arXmlTool.exe merge -a testbench/cases/1.2/multiple1.arxml -a testbench/cases/1.2/multiple2.arxml -a testbench/cases/1.2/multiple3.arxml -m testbench/results/1.2/merged_multiple.arxml
echo ""

echo "Test Case 1.3: Merge Different Structures"
rm -rf testbench/results/1.3/*
echo "Command: ./arXmlTool.exe merge -a testbench/cases/1.3/structure1.arxml -a testbench/cases/1.3/structure2.arxml -m testbench/results/1.3/merged_structure.arxml"
./arXmlTool.exe merge -a testbench/cases/1.3/structure1.arxml -a testbench/cases/1.3/structure2.arxml -m testbench/results/1.3/merged_structure.arxml
echo ""

echo "Test Case 1.4: Merge Non-conflicting Containers"
rm -rf testbench/results/1.4/*
echo "Command: ./arXmlTool.exe merge -a testbench/cases/1.4/noconflict1.arxml -a testbench/cases/1.4/noconflict2.arxml -a testbench/cases/1.4/noconflict3.arxml -m testbench/results/1.4/merged_noconflict.arxml"
./arXmlTool.exe merge -a testbench/cases/1.4/noconflict1.arxml -a testbench/cases/1.4/noconflict2.arxml -a testbench/cases/1.4/noconflict3.arxml -m testbench/results/1.4/merged_noconflict.arxml
echo ""


echo "-------------------"
echo "Test Case 2: Command File Tests"
echo "-------------------"

echo "Test Case 2.1: Command File Input"
rm -rf testbench/results/2.1/*
echo "Command: ./arXmlTool.exe merge -f testbench/cases/2.1/command.txt"
./arXmlTool.exe merge -f testbench/cases/2.1/command.txt
echo ""

echo "Test Case 2.2: Command File With Multiple Lines Input"
rm -rf testbench/results/2.2/*
echo "Command: ./arXmlTool.exe merge -f testbench/cases/2.2/command.txt"
./arXmlTool.exe merge -f testbench/cases/2.2/command.txt
echo ""


echo "-------------------"
echo "Test Case 3: Indentation Tests"
echo "-------------------"

echo "Test Case 3.1: Tab Indentation"
rm -rf testbench/results/3.1/*
echo "Command: ./arXmlTool.exe merge -a testbench/cases/3.1/indent_test.arxml -m testbench/results/3.1/merged_tab.arxml -i tab"
./arXmlTool.exe merge -a testbench/cases/3.1/indent_test.arxml -m testbench/results/3.1/merged_tab.arxml -i tab
echo ""

echo "Test Case 3.2: 2-Space Indentation"
rm -rf testbench/results/3.2/*
echo "Command: ./arXmlTool.exe merge -a testbench/cases/3.2/indent_test.arxml -m testbench/cases/3.2/merged_2space.arxml -i 2"
./arXmlTool.exe merge -a testbench/cases/3.2/indent_test.arxml -m testbench/results/3.2/merged_2space.arxml -i 2
echo ""

echo "Test Case 3.3: 4-Space Indentation (Default)"
rm -rf testbench/results/3.3/*
echo "Command: ./arXmlTool.exe merge -a testbench/cases/3.3/indent_test.arxml -m testbench/cases/3.3/merged_4space.arxml -i 4"
./arXmlTool.exe merge -a testbench/cases/3.3/indent_test.arxml -m testbench/results/3.3/merged_4space.arxml -i 4
echo ""

echo "Test Case 3.4: Invalid Indentation Style"
rm -rf testbench/results/3.4/*
echo "Command: ./arXmlTool.exe merge -a testbench/cases/3.4/indent_test.arxml -m testbench/cases/3.4/merged_invalid.arxml -i 3"
./arXmlTool.exe merge -a testbench/cases/3.4/indent_test.arxml -m testbench/results/3.4/merged_invalid.arxml -i 3
echo ""


echo "-------------------"
echo "Test Case 4: Format Mode Tests"
echo "-------------------"

echo "Test Case 4.1: Format with Tab Indentation (In-place)"
rm -rf testbench/results/4.1/*
echo "Command: ./arXmlTool.exe format -a testbench/results/4.1/format_test_tab.arxml -i tab"
cp testbench/cases/4.1/format_test.arxml testbench/results/4.1/format_test_tab.arxml
./arXmlTool.exe format -a testbench/results/4.1/format_test_tab.arxml -i tab
echo ""

echo "Test Case 4.2: Format with 2-Space Indentation (Output Directory)"
rm -rf testbench/results/4.2/*
echo "Command: ./arXmlTool.exe format -a testbench/cases/4.2/format_test.arxml -i 2 -o testbench/results/4.2"
./arXmlTool.exe format -a testbench/cases/4.2/format_test.arxml -i 2 -o testbench/results/4.2
echo ""

echo "Test Case 4.3: Format Multiple Files"
rm -rf testbench/results/4.3/*
echo "Command: ./arXmlTool.exe format -a testbench/cases/4.3/format_test.arxml -a testbench/cases/4.3/indent_test.arxml -i 4 -o testbench/results/4.3"
./arXmlTool.exe format -a testbench/cases/4.3/format_test.arxml -a testbench/cases/4.3/indent_test.arxml -i 4 -o testbench/results/4.3
echo ""

echo "Test Case 4.4: Format with Invalid Style"
rm -rf testbench/results/4.4/*
echo "Command: ./arXmlTool.exe format -a testbench/cases/4.4/format_test.arxml -i 3"
./arXmlTool.exe format -a testbench/cases/4.4/format_test.arxml -i 3 -o testbench/results/4.4
echo ""
