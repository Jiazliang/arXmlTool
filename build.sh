#!/bin/bash

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