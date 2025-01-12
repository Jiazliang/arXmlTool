# arXmlTool

arXmlTool 是一个用于处理和解析 XML 文件的工具库。

## 项目结构 

```
.
├── arXmlTool.c    # 核心实现文件
├── build.sh       # 编译脚本
└── run_tests.sh   # 测试脚本
```

## 构建和测试

### 编译
```bash
# Windows环境 (MinGW)
./build.sh

# Linux/Unix环境
./build.sh
```

### 运行测试
```bash
# 执行所有测试用例
./run_tests.sh
```

## 命令行使用说明

### 基本语法

```bash
# 直接使用命令行参数
arXmlTool.exe <mode> [options]

# 或者通过命令文件执行
arXmlTool.exe -f <command_file>
```

### 支持的模式（mode）
- `merge`: 合并多个 ARXML 文件
- `compare`: 比较 ARXML 文件
- `generate`: 生成 ARXML 文件

### Merge 模式参数
- `-a <file.arxml>`: 指定输入文件（可多次使用以指定多个输入文件）
- `-m <file.arxml>`: 指定输出文件
- `-o <directory>`: 指定输出目录（可选，默认为当前目录）
- `-i <style>`: 指定输出文件的缩进样式（可选）
  - `tab`: 使用Tab缩进
  - `2`: 使用2空格缩进
  - `4`: 使用4空格缩进（默认）

### 基本使用示例

```bash
# 合并两个文件（使用默认4空格缩进）
arXmlTool.exe merge -a input1.arxml -a input2.arxml -m output.arxml

# 使用Tab缩进合并文件
arXmlTool.exe merge -a input1.arxml -a input2.arxml -m output.arxml -i tab

# 使用2空格缩进并指定输出目录
arXmlTool.exe merge -a input1.arxml -a input2.arxml -m output.arxml -i 2 -o /output/dir

# 使用命令文件
arXmlTool.exe -f command.txt
```

## 注意事项

1. 输入文件必须存在且可读
2. 输出目录必须存在
3. 合并模式至少需要一个输入文件和一个输出文件
4. 输入文件数量不能超过 1024 个
5. 文件路径长度不能超过 256 字符

## 返回值
- 0: 执行成功
- 1: 执行失败（参数错误或操作失败）

## 依赖项
- libxml2 库
- Git Bash（Windows 环境下）

