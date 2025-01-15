# arXmlTool

arXmlTool 是一个用于处理和解析 XML 文件的工具库。

## 项目结构 

```
.
├── src/                    # 源代码目录
│   ├── main/              # 主程序
│   │   ├── arxml_tool.c   # 程序入口
│   │   ├── arxml_tool.h   # 主程序头文件
│   │   ├── common.h       # 通用定义
│   │   ├── options.c      # 选项处理
│   │   └── options.h      # 选项定义
│   ├── command/           # 命令处理
│   │   ├── command.c      # 命令行参数处理
│   │   └── command.h      # 命令行相关定义
│   ├── operations/        # 操作实现
│   │   ├── merge.c        # 合并操作
│   │   ├── merge.h        # 合并接口
│   │   ├── format.c       # 格式化操作
│   │   └── format.h       # 格式化接口
│   └── utils/             # 工具函数
│       ├── xml_utils.c    # XML操作工具
│       └── xml_utils.h    # XML工具接口
├── build/                 # 编译输出目录
├── testbench/            # 测试相关
│   ├── cases/            # 测试用例
│   └── results/          # 测试结果
├── build_and_test.sh     # 编译和测试脚本
└── README.md             # 项目文档
```

## 构建和测试

### 编译
```bash
# Windows环境 (MinGW)
./build_and_test.sh

# Linux/Unix环境
./build_and_test.sh
```

### 运行测试
```bash
# 编译并执行所有测试用例
./build_and_test.sh
```

## 命令行使用说明

### 基本语法

```bash
# 直接使用命令行参数
build/arXmlTool.exe <mode> [options]

# 或者通过命令文件执行
build/arXmlTool.exe <mode> -f <command_file>
```

### 支持的模式（mode）
- `merge`: 合并多个 ARXML 文件
- `compare`: 比较 ARXML 文件（未实现）
- `generate`: 生成 ARXML 文件（未实现）
- `format`: 格式化 ARXML 文件

### Merge 模式参数
- `-a <file.arxml>`: 指定输入文件（可多次使用以指定多个输入文件）
- `-m <file.arxml>`: 指定输出文件
- `-o <directory>`: 指定输出目录（可选）。如果指定了此参数，将忽略-m参数中的目录部分，
                    仅使用其文件名，并将文件保存到-o指定的目录中
- `-i <style>`: 指定输出文件的缩进样式（可选）
  - `tab`: 使用Tab缩进
  - `2`: 使用2空格缩进
  - `4`: 使用4空格缩进（默认）

### Format 模式参数
- `-a <file.arxml>`: 指定输入文件（可多次使用以指定多个输入文件）
- `-o <directory>`: 指定输出目录（可选，默认覆盖源文件）
- `-i <style>`: 指定缩进样式（可选，同merge模式）

### 基本使用示例

```bash
# 合并两个文件（使用默认4空格缩进）
build/arXmlTool.exe merge -a input1.arxml -a input2.arxml -m output.arxml

# 使用Tab缩进合并文件，并指定输出目录（将生成 /output/dir/output.arxml）
build/arXmlTool.exe merge -a input1.arxml -a input2.arxml -m /some/path/output.arxml -i tab -o /output/dir

# 使用2空格缩进并指定输出目录（将生成 /output/dir/result.arxml）
build/arXmlTool.exe merge -a input1.arxml -a input2.arxml -m result.arxml -i 2 -o /output/dir

# 格式化单个文件（覆盖原文件）
build/arXmlTool.exe format -a input.arxml -i tab

# 格式化多个文件并输出到指定目录
build/arXmlTool.exe format -a input1.arxml -a input2.arxml -i 2 -o /output/dir

# 使用命令文件
build/arXmlTool.exe merge -f command.txt
```

## 注意事项

1. 输入文件必须存在且可读
2. 如果指定的输出目录不存在，程序会自动创建（包括多级目录）
3. 合并模式至少需要一个输入文件和一个输出文件
4. 格式化模式至少需要一个输入文件
5. 输入文件数量不能超过 1024 个
6. 文件路径长度不能超过 256 字符

## 返回值
- 0: 执行成功
- 1: 执行失败（参数错误或操作失败）

## 依赖项
- libxml2 库
- MinGW（Windows 环境）或 GCC（Linux/Unix 环境）
- Git Bash（Windows 环境下）

