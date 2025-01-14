/* Use static library | 使用静态库 */
#define LIBXML_STATIC  

/* Include standard libraries | 包含标准库 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

/* Platform-specific includes | 平台特定的包含 */
#ifdef _WIN32
/* For _access function in Windows | Windows系统下的文件访问函数 */
#include <io.h>     
#else
/* For access function in Unix systems | Unix系统下的文件访问函数 */
#include <unistd.h> 
#endif

/* Constants definition | 常量定义 */
#define MAX_FILES 1024
#define MAX_PATH 256
#define MAX_CMD_LENGTH 4096
#define MAX_BUFFER_SIZE 8192

/* Operation modes enumeration | 操作模式枚举 */
typedef enum {
    MODE_MERGE,    /* Merge mode | 合并模式 */
    MODE_COMPARE,  /* Compare mode | 比较模式 */
    MODE_GENERATE, /* Generate mode | 生成模式 */
    MODE_FORMAT,   /* Format mode | 格式化模式 */
    MODE_UNKNOWN   /* Unknown mode | 未知模式 */
} OperationMode;

/* Program options structure | 程序选项结构 */
typedef struct {
    OperationMode mode;
    char input_files[MAX_FILES][MAX_PATH];
    int input_file_count;
    char output_file[MAX_PATH];
    char output_dir[MAX_PATH];
    enum {
        INDENT_TAB,    /* Tab indentation | Tab缩进 */
        INDENT_2SPACE, /* 2 spaces indentation | 2空格缩进 */
        INDENT_4SPACE  /* 4 spaces indentation | 4空格缩进 */
    } indent_style;
} ProgramOptions;

/* Function declarations | 函数声明 */
static xmlChar* get_short_name(xmlNodePtr node);
static void merge_node(xmlNodePtr base_parent, xmlNodePtr input_node, xmlDocPtr doc);

OperationMode parse_mode(const char* mode_str) {
    if (strcmp(mode_str, "merge") == 0) return MODE_MERGE;
    if (strcmp(mode_str, "compare") == 0) return MODE_COMPARE;
    if (strcmp(mode_str, "generate") == 0) return MODE_GENERATE;
    if (strcmp(mode_str, "format") == 0) return MODE_FORMAT;
    return MODE_UNKNOWN;
}

void print_usage() {
    printf("Usage:\n");
    printf("  arXmlTool.exe <mode> [options]\n");
    printf("  arXmlTool.exe -f <command_file>\n\n");
    printf("Modes:\n");
    printf("  merge    - Merge multiple ARXML files\n");
    printf("  compare  - Compare ARXML files\n");
    printf("  generate - Generate ARXML file\n");
    printf("  format   - Format ARXML files\n\n");
    printf("Merge mode options:\n");
    printf("  -a <file.arxml>  Specify input file (can be used multiple times)\n");
    printf("  -m <file.arxml>  Specify output file\n");
    printf("  -o <directory>   Specify output directory (optional)\n");
    printf("  -i <style>       Specify indentation style (optional)\n");
    printf("                   - 'tab': Use tab for indentation\n");
    printf("                   - '2': Use 2 spaces for indentation\n");
    printf("                   - '4': Use 4 spaces for indentation (default)\n\n");
    printf("Format mode options:\n");
    printf("  -a <file.arxml>  Specify input file (can be used multiple times)\n");
    printf("  -o <directory>   Specify output directory (optional, will overwrite source files if not specified)\n");
    printf("  -i <style>       Specify indentation style (optional)\n");
    printf("                   - 'tab': Use tab for indentation\n");
    printf("                   - '2': Use 2 spaces for indentation\n");
    printf("                   - '4': Use 4 spaces for indentation (default)\n");
}

/* Parse merge mode options | 解析合并模式的选项 */
int parse_merge_options(int argc, char *argv[], ProgramOptions *opts) {
    int opt;
    opts->input_file_count = 0;
    opts->indent_style = INDENT_4SPACE; /* Default to 4 spaces indentation | 默认使用4空格缩进 */
    
    /* Set default output directory to current directory | 设置默认输出目录为当前目录 */
    strncpy(opts->output_dir, ".", MAX_PATH - 1);
    
    /* Parse command line options | 解析命令行选项 */
    while ((opt = getopt(argc, argv, "a:m:o:i:")) != -1) {
        switch (opt) {
            case 'a':
                /* Check if maximum file limit is reached | 检查是否达到最大文件数限制 */
                if (opts->input_file_count >= MAX_FILES) {
                    printf("Error: Number of input files exceeds limit (%d)\n", MAX_FILES);
                    return 0;
                }
                /* Add input file to list | 添加输入文件到列表 */
                strncpy(opts->input_files[opts->input_file_count], optarg, MAX_PATH - 1);
                opts->input_file_count++;
                break;
            case 'm':
                /* Set output file path | 设置输出文件路径 */
                strncpy(opts->output_file, optarg, MAX_PATH - 1);
                break;
            case 'o':
                /* Set output directory path | 设置输出目录路径 */
                strncpy(opts->output_dir, optarg, MAX_PATH - 1);
                break;
            case 'i':
                /* Parse indentation style | 解析缩进风格 */
                if (strcmp(optarg, "tab") == 0) {
                    opts->indent_style = INDENT_TAB;
                } else if (strcmp(optarg, "2") == 0) {
                    opts->indent_style = INDENT_2SPACE;
                } else if (strcmp(optarg, "4") == 0) {
                    opts->indent_style = INDENT_4SPACE;
                } else {
                    printf("Error: Invalid indent style '%s'. Use 'tab', '2' or '4'\n", optarg);
                    return 0;
                }
                break;
            case '?':
                /* Invalid option or missing argument | 无效选项或缺少参数 */
                printf("Error: Invalid option or missing argument\n");
                return 0;
            default:
                /* Unknown option | 未知选项 */
                printf("Error: Unknown option %c\n", opt);
                return 0;
        }
    }
    
    /* Check required parameters | 检查必需的参数 */
    if (opts->input_file_count == 0 || strlen(opts->output_file) == 0) {
        printf("Error: Merge mode requires at least one input file (-a) and one output file (-m)\n");
        printf("Debug: input_file_count = %d, output_file = '%s'\n", 
               opts->input_file_count, opts->output_file);
        return 0;
    }
    
    /* Check if input files exist | 检查输入文件是否存在 */
    for (int i = 0; i < opts->input_file_count; i++) {
        FILE* file = fopen(opts->input_files[i], "r");
        if (!file) {
            printf("Error: Cannot open input file '%s'\n", opts->input_files[i]);
            return 0;
        }
        fclose(file);
    }
    
    /* Check if output directory exists | 检查输出目录是否存在 */
    if (strcmp(opts->output_dir, ".") != 0) {
        #ifdef _WIN32
        if (_access(opts->output_dir, 0) != 0) {
            printf("Error: Output directory '%s' does not exist\n", opts->output_dir);
            return 0;
        }
        #else
        if (access(opts->output_dir, F_OK) != 0) {
            printf("Error: Output directory '%s' does not exist\n", opts->output_dir);
            return 0;
        }
        #endif
    }
    
    return 1;
}

/* Read command line from file | 从文件读取命令行参数 */
char** read_command_from_file(const char* filename, int* argc) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open command file '%s'\n", filename);
        return NULL;
    }

    /* Read all lines and merge them into one line | 读取所有行并合并成一行 */
    char buffer[MAX_BUFFER_SIZE] = {0};
    char line[MAX_CMD_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        /* Remove trailing newline | 移除行尾的换行符 */
        line[strcspn(line, "\r\n")] = 0;
        
        /* Skip empty lines | 跳过空行 */
        if (strlen(line) == 0) {
            continue;
        }
        
        /* Add space as separator if buffer is not empty | 如果buffer不为空，添加空格作为分隔符 */
        if (strlen(buffer) > 0) {
            strcat(buffer, " ");
        }
        
        /* Check if buffer has enough space | 检查buffer是否有足够空间 */
        if (strlen(buffer) + strlen(line) >= MAX_BUFFER_SIZE - 1) {
            printf("Error: Command line too long\n");
            fclose(file);
            return NULL;
        }
        
        /* Append current line to buffer | 添加当前行到buffer */
        strcat(buffer, line);
    }
    fclose(file);

    if (strlen(buffer) == 0) {
        printf("Error: Command file is empty\n");
        return NULL;
    }

    /* Count arguments | 计算参数个数 */
    int count = 1; /* Program name takes first position | 程序名占用第一个位置 */
    char* p = buffer;
    while (*p) {
        if (*p == ' ' || *p == '\t') {
            count++;
            while (*p && (*p == ' ' || *p == '\t')) p++;
        } else {
            p++;
        }
    }

    /* Allocate memory for argument array | 为参数数组分配内存 */
    char** argv = (char**)malloc((count + 1) * sizeof(char*));
    if (!argv) {
        printf("Error: Memory allocation failed\n");
        return NULL;
    }

    /* Set program name as first argument | 设置程序名为第一个参数 */
    argv[0] = strdup("arXmlTool.exe");

    /* Split command line into arguments | 将命令行分割为参数 */
    int i = 1;
    char* token = strtok(buffer, " \t");
    while (token) {
        argv[i] = strdup(token);
        token = strtok(NULL, " \t");
        i++;
    }
    argv[i] = NULL;
    *argc = i;

    return argv;
}

/* Free command line arguments memory | 释放命令行参数内存 */
void free_command_args(char** argv) {
    if (!argv) return;
    for (int i = 0; argv[i] != NULL; i++) {
        free(argv[i]);
    }
    free(argv);
}

/* Compare node names ignoring namespaces | 比较节点名称（忽略命名空间） */
static int compare_node_names(const xmlChar* name1, const xmlChar* name2) {
    /* Skip namespace prefix | 跳过命名空间前缀 */
    const xmlChar* local_name1 = xmlStrchr(name1, ':');
    const xmlChar* local_name2 = xmlStrchr(name2, ':');
    
    /* If no namespace prefix, use full name | 如果没有命名空间前缀，使用完整名称 */
    if (local_name1 == NULL) local_name1 = name1;
    else local_name1++; /* Skip colon | 跳过冒号 */
    if (local_name2 == NULL) local_name2 = name2;
    else local_name2++; /* Skip colon | 跳过冒号 */
    
    return xmlStrcmp(local_name1, local_name2);
}

/* Merge ARXML files implementation | ARXML文件合并实现 */
int merge_arxml_files(const ProgramOptions *opts) {
    xmlDocPtr base_doc = NULL;
    xmlNodePtr root_node = NULL;
    
    /* Parse base file | 解析基础文件 */
    base_doc = xmlReadFile(opts->input_files[0], NULL, XML_PARSE_NOBLANKS | XML_PARSE_NOBLANKS | XML_PARSE_COMPACT);
    if (base_doc == NULL) {
        printf("Error: Cannot parse base file '%s'\n", opts->input_files[0]);
        return 0;
    }
    
    /* Get root node | 获取根节点 */
    root_node = xmlDocGetRootElement(base_doc);
    if (root_node == NULL) {
        printf("Error: File '%s' is empty\n", opts->input_files[0]);
        xmlFreeDoc(base_doc);
        return 0;
    }
    
    /* Process other files | 处理其他文件 */
    for (int i = 1; i < opts->input_file_count; i++) {
        xmlDocPtr doc = xmlReadFile(opts->input_files[i], NULL, XML_PARSE_NOBLANKS);
        if (doc == NULL) {
            printf("Error: Cannot parse file '%s'\n", opts->input_files[i]);
            xmlFreeDoc(base_doc);
            return 0;
        }
        
        xmlNodePtr cur_root = xmlDocGetRootElement(doc);
        if (cur_root == NULL) {
            printf("Error: File '%s' is empty\n", opts->input_files[i]);
            xmlFreeDoc(doc);
            xmlFreeDoc(base_doc);
            return 0;
        }
        
        /* Recursively merge nodes | 递归合并节点 */
        xmlNodePtr cur = cur_root->children;
        while (cur != NULL) {
            merge_node(root_node, cur, base_doc);
            cur = cur->next;
        }
        
        xmlFreeDoc(doc);
    }
    
    /* Set output format based on indentation style | 根据缩进样式设置输出格式 */
    int format_output = 1;  /* Enable formatted output | 启用格式化输出 */
    switch (opts->indent_style) {
        case INDENT_TAB:
            xmlTreeIndentString = "\t";
            break;
        case INDENT_2SPACE:
            xmlTreeIndentString = "  ";
            break;
        case INDENT_4SPACE:
            xmlTreeIndentString = "    ";
            break;
        default:
            xmlTreeIndentString = "    "; /* Default to 4 spaces | 默认使用4空格 */
    }

    /* Configure output settings | 配置输出设置 */
    xmlKeepBlanksDefault(0);
    xmlIndentTreeOutput = 1;

    /* Save document | 保存文档 */
    if (xmlSaveFormatFileEnc(opts->output_file, base_doc, "UTF-8", format_output) < 0) {
        printf("Error: Cannot save output file '%s'\n", opts->output_file);
        xmlFreeDoc(base_doc);
        return 0;
    }
    
    xmlFreeDoc(base_doc);
    /* Print completion message | 打印完成消息 */
    if (opts->input_file_count > 1) {
        printf("Merge completed, output file: %s\n", opts->output_file);
    }
    return 1;
}

/* Count SHORT-NAME nodes in node and all its children | 统计节点及其所有子节点中的SHORT-NAME节点数量 */
static int count_short_names(xmlNodePtr node) {
    /* If not an element node, return 0 | 如果不是元素节点，返回0 */
    if (node->type != XML_ELEMENT_NODE) {
        return 0;
    }
    
    int count = 0;
    xmlNodePtr cur = node->children;
    
    /* Recursively check all child nodes | 递归检查所有子节点 */
    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"SHORT-NAME"))) {
            count++;
            if (count > 1) {
                return count;  /* If multiple found, return immediately | 如果找到多个，立即返回 */
            }
        } else if (cur->type == XML_ELEMENT_NODE) {
            count += count_short_names(cur);
            if (count > 1) {
                return count;  /* If multiple found, return immediately | 如果找到多个，立即返回 */
            }
        }
        cur = cur->next;
    }
    
    return count;
}

/* Recursively merge nodes | 递归合并节点 */
static void merge_node(xmlNodePtr base_parent, xmlNodePtr input_node, xmlDocPtr doc) {
    /* Skip text nodes and comment nodes | 跳过文本节点和注释节点 */
    if (input_node->type != XML_ELEMENT_NODE) {
        return;
    }

    xmlChar* input_name = get_short_name(input_node);
    xmlNodePtr existing_node = NULL;
    
    /* Search for nodes of the same type in base_parent's children | 在基础父节点的子节点中搜索相同类型的节点 */
    xmlNodePtr cur = base_parent->children;
    while (cur != NULL) {
        /* Compare only nodes of the same type, ignore namespaces | 只比较相同类型的节点，忽略命名空间 */
        if (compare_node_names(cur->name, input_node->name) == 0) {
            xmlChar* cur_name = get_short_name(cur);
            
            /* If both nodes have SHORT-NAME, compare names | 如果两个节点都有SHORT-NAME，比较名称 */
            if (input_name != NULL && cur_name != NULL) {
                if (!xmlStrcmp(cur_name, input_name)) {
                    existing_node = cur;
                }
            } 
            /* If neither node has SHORT-NAME, consider them matching | 如果两个节点都没有SHORT-NAME，认为它们匹配 */
            else if (input_name == NULL && cur_name == NULL) {
                existing_node = cur;
            }
            
            if (cur_name != NULL) {
                xmlFree(cur_name);
            }
            
            if (existing_node) {
                break;
            }
        }
        cur = cur->next;
    }
    
    if (input_name != NULL) {
        xmlFree(input_name);
    }
    
    if (existing_node != NULL) {
        /* Found matching node | 找到匹配的节点 */
        if (input_name != NULL && count_short_names(input_node) == 1) {
            /* If it's a minimal unit with SHORT-NAME, discard entire subtree | 如果是带有SHORT-NAME的最小单元，丢弃整个子树 */
            return;
        }
        /* Otherwise process child nodes recursively | 否则递归处理子节点 */
        xmlNodePtr input_child = input_node->children;
        while (input_child != NULL) {
            merge_node(existing_node, input_child, doc);
            input_child = input_child->next;
        }
        return;
    }
    
    /* No matching node found, copy entire subtree | 未找到匹配节点，复制整个子树 */
    xmlNodePtr copy = xmlNewNode(NULL, input_node->name);
    
    xmlNodePtr child = input_node->children;
    while (child != NULL) {
        if (child->type == XML_ELEMENT_NODE) {
            /* Recursively copy element nodes | 递归复制元素节点 */
            merge_node(copy, child, doc);
        } else if (child->type == XML_TEXT_NODE) {
            /* Copy text content | 复制文本内容 */
            xmlChar* content = xmlNodeGetContent(child);
            xmlNodePtr text = xmlNewText(content);
            xmlAddChild(copy, text);
            xmlFree(content);
        }
        child = child->next;
    }
    xmlAddChild(base_parent, copy);
}

/* Get SHORT-NAME content from node | 从节点获取SHORT-NAME内容 */
static xmlChar* get_short_name(xmlNodePtr node) {
    xmlNodePtr cur = node->children;
    while (cur != NULL) {
        /* Find SHORT-NAME node and return its content | 查找SHORT-NAME节点并返回其内容 */
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"SHORT-NAME"))) {
            return xmlNodeGetContent(cur);
        }
        cur = cur->next;
    }
    return NULL;
}

/* Parse format mode options | 解析格式化模式的选项 */
int parse_format_options(int argc, char *argv[], ProgramOptions *opts) {
    int opt;
    opts->input_file_count = 0;
    opts->indent_style = INDENT_4SPACE; /* Default to 4 spaces indentation | 默认使用4空格缩进 */
    
    /* Set default output directory to current directory | 设置默认输出目录为当前目录 */
    strncpy(opts->output_dir, ".", MAX_PATH - 1);
    
    /* Parse command line options | 解析命令行选项 */
    while ((opt = getopt(argc, argv, "a:o:i:")) != -1) {
        switch (opt) {
            case 'a':
                /* Check if maximum file limit is reached | 检查是否达到最大文件数限制 */
                if (opts->input_file_count >= MAX_FILES) {
                    printf("Error: Number of input files exceeds limit (%d)\n", MAX_FILES);
                    return 0;
                }
                /* Add input file to list | 添加输入文件到列表 */
                strncpy(opts->input_files[opts->input_file_count], optarg, MAX_PATH - 1);
                opts->input_file_count++;
                break;
            case 'o':
                /* Set output directory path | 设置输出目录路径 */
                strncpy(opts->output_dir, optarg, MAX_PATH - 1);
                break;
            case 'i':
                /* Parse indentation style | 解析缩进风格 */
                if (strcmp(optarg, "tab") == 0) {
                    opts->indent_style = INDENT_TAB;
                } else if (strcmp(optarg, "2") == 0) {
                    opts->indent_style = INDENT_2SPACE;
                } else if (strcmp(optarg, "4") == 0) {
                    opts->indent_style = INDENT_4SPACE;
                } else {
                    printf("Error: Invalid indent style '%s'. Use 'tab', '2' or '4'\n", optarg);
                    return 0;
                }
                break;
            case '?':
                /* Invalid option or missing argument | 无效选项或缺少参数 */
                printf("Error: Invalid option or missing argument\n");
                return 0;
            default:
                /* Unknown option | 未知选项 */
                printf("Error: Unknown option %c\n", opt);
                return 0;
        }
    }
    
    /* Check required parameters | 检查必需的参数 */
    if (opts->input_file_count == 0) {
        printf("Error: Format mode requires at least one input file (-a)\n");
        return 0;
    }
    
    /* Check if input files exist | 检查输入文件是否存在 */
    for (int i = 0; i < opts->input_file_count; i++) {
        FILE* file = fopen(opts->input_files[i], "r");
        if (!file) {
            printf("Error: Cannot open input file '%s'\n", opts->input_files[i]);
            return 0;
        }
        fclose(file);
    }
    
    /* Check if output directory exists | 检查输出目录是否存在 */
    if (strcmp(opts->output_dir, ".") != 0) {
        #ifdef _WIN32
        if (_access(opts->output_dir, 0) != 0) {
            printf("Error: Output directory '%s' does not exist\n", opts->output_dir);
            return 0;
        }
        #else
        if (access(opts->output_dir, F_OK) != 0) {
            printf("Error: Output directory '%s' does not exist\n", opts->output_dir);
            return 0;
        }
        #endif
    }
    
    return 1;
}

/* Format ARXML files implementation | ARXML文件格式化实现 */
int format_arxml_files(const ProgramOptions *opts) {
    /* Set global formatting parameters | 设置全局格式化参数 */
    xmlKeepBlanksDefault(0);
    xmlIndentTreeOutput = 1;
    int format_output = 1;

    /* Set indentation string based on options | 根据选项设置缩进字符串 */
    const char* indent_str;
    switch (opts->indent_style) {
        case INDENT_TAB:
            indent_str = "\t";
            break;
        case INDENT_2SPACE:
            indent_str = "  ";
            break;
        case INDENT_4SPACE:
            indent_str = "    ";
            break;
        default:
            indent_str = "    ";
    }

    /* Process each input file | 处理每个输入文件 */
    for (int i = 0; i < opts->input_file_count; i++) {
        /* Parse XML file | 解析XML文件 */
        xmlDocPtr doc = xmlReadFile(opts->input_files[i], NULL, XML_PARSE_NOBLANKS);
        if (doc == NULL) {
            printf("Error: Cannot parse file '%s'\n", opts->input_files[i]);
            return 0;
        }

        /* Determine output path | 确定输出路径 */
        const char* output_path;
        char new_path[MAX_PATH];
        
        if (strcmp(opts->output_dir, ".") == 0) {
            /* If no output directory specified, overwrite source file | 如果没有指定输出目录，直接覆盖源文件 */
            output_path = opts->input_files[i];
        } else {
            /* Extract filename from input path | 从输入路径中提取文件名 */
            const char* input_filename = strrchr(opts->input_files[i], '/');
            if (input_filename == NULL) {
                input_filename = strrchr(opts->input_files[i], '\\');
            }
            if (input_filename == NULL) {
                input_filename = opts->input_files[i];
            } else {
                input_filename++; /* Skip slash | 跳过斜杠 */
            }
            
            /* Safely construct output path | 安全地构建输出路径 */
            size_t dir_len = strlen(opts->output_dir);
            size_t file_len = strlen(input_filename);
            
            /* Check path length | 检查路径长度 */
            if (dir_len + file_len + 2 > MAX_PATH) { /* +2 for '/' and '\0' */
                printf("Error: Output path too long for file '%s'\n", input_filename);
                xmlFreeDoc(doc);
                return 0;
            }
            
            /* Copy directory path | 复制目录路径 */
            strncpy(new_path, opts->output_dir, MAX_PATH - 1);
            new_path[MAX_PATH - 1] = '\0';
            
            /* Ensure directory path ends with slash | 确保目录路径以斜杠结尾 */
            if (dir_len > 0 && new_path[dir_len - 1] != '/' && new_path[dir_len - 1] != '\\') {
                if (dir_len + 1 < MAX_PATH) {
                    new_path[dir_len] = '/';
                    new_path[dir_len + 1] = '\0';
                    dir_len++;
                }
            }
            
            /* Safely append filename | 安全地添加文件名 */
            if (dir_len + file_len < MAX_PATH) {
                strcpy(new_path + dir_len, input_filename);
                output_path = new_path;
            } else {
                printf("Error: Output path too long\n");
                xmlFreeDoc(doc);
                return 0;
            }
        }

        /* Set indentation string before processing each file | 在处理每个文件前设置缩进字符串 */
        xmlTreeIndentString = indent_str;

        /* Save formatted file | 保存格式化后的文件 */
        if (xmlSaveFormatFileEnc(output_path, doc, "UTF-8", format_output) < 0) {
            printf("Error: Cannot save formatted file '%s'\n", output_path);
            xmlFreeDoc(doc);
            return 0;
        }

        xmlFreeDoc(doc);
        
        /* Print appropriate message based on output location | 根据输出位置打印相应的消息 */
        if (strcmp(opts->output_dir, ".") == 0) {
            printf("File formatted in place: %s\n", output_path);
        } else {
            printf("Formatted file saved to: %s\n", output_path);
        }
    }

    return 1;
}

/* Program entry point | 程序入口点 */
int main(int argc, char *argv[]) {
    /* Initialize libxml2 | 初始化libxml2库 */
    LIBXML_TEST_VERSION
    
    char** cmd_argv = NULL;
    int cmd_argc = 0;
    int using_cmd_file = 0;

    if (argc < 2) {
        print_usage();
        return 1;
    }

    /* Parse operation mode | 解析操作模式 */
    ProgramOptions opts = {0};
    opts.mode = parse_mode(argv[1]);

    if (opts.mode == MODE_UNKNOWN) {
        printf("Error: Unknown operation mode '%s'\n", argv[1]);
        print_usage();
        return 1;
    }

    /* Check if using command file | 检查是否使用命令文件 */
    if (argc == 4 && strcmp(argv[2], "-f") == 0) {
        cmd_argv = read_command_from_file(argv[3], &cmd_argc);
        if (!cmd_argv) {
            return 1;
        }
        /* Use command file arguments directly | 直接使用命令文件中的参数 */
        argv = cmd_argv;
        argc = cmd_argc;
        using_cmd_file = 1;
    }

    int success = 0;
    switch (opts.mode) {
        case MODE_MERGE: {
            /* Reset getopt before parsing options | 在解析选项前重置getopt */
            optind = 1;
            
            if (using_cmd_file) {
                /* Use command file arguments directly | 直接使用命令文件参数 */
                success = parse_merge_options(argc, argv, &opts);
            } else {
                /* Skip mode name for command line arguments | 对于命令行参数跳过模式名 */
                success = parse_merge_options(argc - 1, argv + 1, &opts);
            }
            
            if (success) {
                printf("Merge mode:\n");
                for (int i = 0; i < opts.input_file_count; i++) {
                    printf("Input file %d: %s\n", i + 1, opts.input_files[i]);
                }
                success = merge_arxml_files(&opts);
            }
            break;
        }
        case MODE_COMPARE:
            printf("Compare mode not implemented\n");
            break;
        case MODE_GENERATE:
            printf("Generate mode not implemented\n");
            break;
        case MODE_FORMAT: {
            /* Reset getopt before parsing options | 在解析选项前重置getopt */
            optind = 1;
            
            if (using_cmd_file) {
                /* Use command file arguments directly | 直接使用命令文件参数 */
                success = parse_format_options(argc, argv, &opts);
            } else {
                /* Skip mode name for command line arguments | 对于命令行参数跳过模式名 */
                success = parse_format_options(argc - 1, argv + 1, &opts);
            }
            
            if (success) {
                printf("Format mode:\n");
                for (int i = 0; i < opts.input_file_count; i++) {
                    printf("Processing file %d: %s\n", i + 1, opts.input_files[i]);
                }
                success = format_arxml_files(&opts);
            }
            break;
        }
        default:
            success = 0;
            break;
    }

    if (!success) {
        print_usage();
        if (using_cmd_file) {
            free_command_args(cmd_argv);
        }
        return 1;
    }

    /* Free command file resources if used | 如果使用了命令文件则释放资源 */
    if (using_cmd_file) {
        free_command_args(cmd_argv);
    }
    
    /* Cleanup libxml2 | 清理libxml2库 */
    xmlCleanupParser();
    
    return 0;
} 