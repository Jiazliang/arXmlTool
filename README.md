# arXmlTool

arXmlTool 是一个用于处理和解析 XML 文件的工具库。

## 项目结构 

```
.
├── arXmlTool.c    # 核心实现文件
├── build.sh       # 编译脚本
└── run_all.sh     # 运行测试脚本
```

## 构建和测试

### Windows 环境下的命令行操作

在 Windows 系统中，可以使用以下方式执行脚本：

#### 1. 使用 PowerShell

```powershell
# 方式一：直接调用可执行文件
.\arXmlTool.exe merge -a input1.arxml -a input2.arxml -m output.arxml

# 方式二：使用 Start-Process
Start-Process -FilePath ".\arXmlTool.exe" -ArgumentList "merge -a input1.arxml -a input2.arxml -m output.arxml"

# 方式三：使用别名（可以在 PowerShell 配置文件中设置）
Set-Alias -Name arxml -Value ".\arXmlTool.exe"
arxml merge -a input1.arxml -a input2.arxml -m output.arxml
```

**PowerShell 使用技巧：**
1. 可以使用 Tab 键自动补全文件名
2. 使用 `Get-Help` 查看命令帮助：
   ```powershell
   Get-Help .\arXmlTool.exe
   ```
3. 设置持久化别名，在 PowerShell 配置文件中添加：
   ```powershell
   # 在 $PROFILE 文件中添加
   Set-Alias -Name arxml -Value "完整路径\arXmlTool.exe"
   ```

#### 2. 使用 PowerShell + bash

```bash
# 方式一：使用完整路径调用 bash
"C:\Program Files\Git\bin\bash.exe" .\build.sh
"C:\Program Files\Git\bin\bash.exe" .\run_all.sh

# 方式二：如果 Git 已添加到环境变量，可以直接使用 bash
bash .\build.sh
bash .\run_all.sh

# 方式三：使用 sh 命令
sh .\build.sh
sh .\run_all.sh
```

#### 3. 使用 Git Bash 终端

```bash
# 直接在 Git Bash 终端中执行
./build.sh
./run_all.sh
```

**注意事项：**
1. 确保已安装 Git（包含 Git Bash）
2. 如果使用方式一，请根据实际 Git 安装路径调整命令
3. 如果使用方式二和方式三，需要将 Git 的 bin 目录添加到系统环境变量 PATH 中
4. 如果脚本无法执行，可能需要添加执行权限：
   ```bash
   chmod +x build.sh
   chmod +x run_all.sh
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

### 使用示例

#### 1. 合并 ARXML 文件

```bash
# 合并两个文件
arXmlTool.exe merge -a input1.arxml -a input2.arxml -m output.arxml

# 指定输出目录
arXmlTool.exe merge -a input1.arxml -a input2.arxml -m output.arxml -o /output/dir
```

#### 2. 使用命令文件

command.txt 内容示例：
```bash
merge -a input1.arxml -a input2.arxml -m output.arxml
```

执行命令：
```bash
arXmlTool.exe -f command.txt
```

## 技术实现细节

### 核心数据结构
```c
typedef enum {
    MODE_MERGE,     // 合并模式
    MODE_COMPARE,   // 比较模式
    MODE_GENERATE,  // 生成模式
    MODE_UNKNOWN    // 未知模式
} OperationMode;

typedef struct {
    OperationMode mode;                    // 操作模式
    char input_files[MAX_FILES][MAX_PATH]; // 输入文件列表
    int input_file_count;                  // 输入文件数量
    char output_file[MAX_PATH];            // 输出文件
    char output_dir[MAX_PATH];             // 输出目录
} ProgramOptions;
```

### 合并策略
1. 基于 SHORT-NAME 进行节点匹配
   - 递归搜索节点中的 SHORT-NAME 元素
   - 比较节点类型和 SHORT-NAME 值确定匹配
2. 递归合并 XML 树结构
   - 对于匹配的节点，递归处理其子节点
   - 对于不匹配的节点，复制整个子树
3. 保持 XML 层次结构完整性
   - 保留命名空间信息
   - 维护节点间的父子关系

### 命令处理流程
1. 命令行解析
   - 直接命令行参数模式
   - 命令文件模式（-f 选项）
2. 参数验证
   - 检查输入文件存在性
   - 验证输出目录权限
   - 确保参数完整性
3. XML 处理
   - 使用 libxml2 库解析 XML
   - 递归合并节点树
   - 格式化输出结果

### 详细使用示例

#### 1. 基本合并操作

```bash
# 合并两个 ARXML 文件
arXmlTool.exe merge -a ECU_Config1.arxml -a ECU_Config2.arxml -m merged_config.arxml

# 合并多个文件并指定输出目录
arXmlTool.exe merge \
    -a Base_Config.arxml \
    -a Network_Config.arxml \
    -a Safety_Config.arxml \
    -m Final_Config.arxml \
    -o C:/Project/Output
```

#### 2. 使用命令文件

command.txt 内容示例：
```bash
# 单行命令格式
merge -a ECU_Config1.arxml -a ECU_Config2.arxml -m merged_config.arxml -o output_dir
```

执行命令：
```bash
# 使用命令文件执行
arXmlTool.exe -f command.txt
```

#### 3. PowerShell 批处理示例

```powershell
# 批量处理多个配置文件
$configs = Get-ChildItem -Filter "*_Config.arxml"
$merged = "merged_result.arxml"

# 构建命令参数
$args = "merge "
foreach ($config in $configs) {
    $args += "-a $($config.Name) "
}
$args += "-m $merged"

# 执行合并
.\arXmlTool.exe $args
```

### 错误处理
工具会在以下情况返回错误：
1. 输入文件不存在或无法访问
2. 输出目录不存在或无权限
3. XML 解析错误
4. 内存分配失败
5. 参数格式错误

### 限制条件
- 最大输入文件数：1024 个
- 文件路径长度上限：256 字符
- 命令长度上限：4096 字符
- 缓冲区大小：8192 字节

### 主要特性
- 支持处理带命名空间的 XML
- 智能节点合并
- 保持 XML 格式化输出
- UTF-8 编码支持

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

