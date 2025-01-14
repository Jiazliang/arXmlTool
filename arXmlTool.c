#define LIBXML_STATIC  // Use static library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#ifdef _WIN32
#include <io.h>     // For _access function in Windows
#else
#include <unistd.h> // For access function in Unix systems
#endif

#define MAX_FILES 1024
#define MAX_PATH 256
#define MAX_CMD_LENGTH 4096
#define MAX_BUFFER_SIZE 8192

typedef enum {
    MODE_MERGE,
    MODE_COMPARE,
    MODE_GENERATE,
    MODE_FORMAT,
    MODE_UNKNOWN
} OperationMode;

typedef struct {
    OperationMode mode;
    char input_files[MAX_FILES][MAX_PATH];
    int input_file_count;
    char output_file[MAX_PATH];
    char output_dir[MAX_PATH];
    enum {
        INDENT_TAB,    // Tab缩进
        INDENT_2SPACE, // 2空格缩进
        INDENT_4SPACE  // 4空格缩进
    } indent_style;
} ProgramOptions;

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

int parse_merge_options(int argc, char *argv[], ProgramOptions *opts) {
    int opt;
    opts->input_file_count = 0;
    opts->indent_style = INDENT_4SPACE; // 默认使用4空格缩进
    
    // Set default output directory to current directory
    strncpy(opts->output_dir, ".", MAX_PATH - 1);
    
    while ((opt = getopt(argc, argv, "a:m:o:i:")) != -1) {
        switch (opt) {
            case 'a':
                if (opts->input_file_count >= MAX_FILES) {
                    printf("Error: Number of input files exceeds limit (%d)\n", MAX_FILES);
                    return 0;
                }
                strncpy(opts->input_files[opts->input_file_count], optarg, MAX_PATH - 1);
                opts->input_file_count++;
                break;
            case 'm':
                strncpy(opts->output_file, optarg, MAX_PATH - 1);
                break;
            case 'o':
                strncpy(opts->output_dir, optarg, MAX_PATH - 1);
                break;
            case 'i':
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
                printf("Error: Invalid option or missing argument\n");
                return 0;
            default:
                printf("Error: Unknown option %c\n", opt);
                return 0;
        }
    }
    
    // Check required parameters: at least one input file and one output file
    if (opts->input_file_count == 0 || strlen(opts->output_file) == 0) {
        printf("Error: Merge mode requires at least one input file (-a) and one output file (-m)\n");
        printf("Debug: input_file_count = %d, output_file = '%s'\n", 
               opts->input_file_count, opts->output_file);
        return 0;
    }
    
    // Check if files exist
    for (int i = 0; i < opts->input_file_count; i++) {
        FILE* file = fopen(opts->input_files[i], "r");
        if (!file) {
            printf("Error: Cannot open input file '%s'\n", opts->input_files[i]);
            return 0;
        }
        fclose(file);
    }
    
    // Check if output directory exists
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

// New: Read command line from file
char** read_command_from_file(const char* filename, int* argc) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open command file '%s'\n", filename);
        return NULL;
    }

    // 读取所有行并合并成一行
    char buffer[MAX_BUFFER_SIZE] = {0};
    char line[MAX_CMD_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        // 移除行尾的换行符
        line[strcspn(line, "\r\n")] = 0;
        
        // 跳过空行
        if (strlen(line) == 0) {
            continue;
        }
        
        // 如果buffer不为空，添加一个空格作为分隔符
        if (strlen(buffer) > 0) {
            strcat(buffer, " ");
        }
        
        // 检查buffer是否有足够空间
        if (strlen(buffer) + strlen(line) >= MAX_BUFFER_SIZE - 1) {
            printf("Error: Command line too long\n");
            fclose(file);
            return NULL;
        }
        
        // 添加当前行到buffer
        strcat(buffer, line);
    }
    fclose(file);

    if (strlen(buffer) == 0) {
        printf("Error: Command file is empty\n");
        return NULL;
    }

    // Count arguments
    int count = 1; // Program name takes first position
    char* p = buffer;
    while (*p) {
        if (*p == ' ' || *p == '\t') {
            count++;
            while (*p && (*p == ' ' || *p == '\t')) p++;
        } else {
            p++;
        }
    }

    // Allocate memory
    char** argv = (char**)malloc((count + 1) * sizeof(char*));
    if (!argv) {
        printf("Error: Memory allocation failed\n");
        return NULL;
    }

    // First argument is program name
    argv[0] = strdup("arXmlTool.exe");

    // Split command line
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

// New: Free command line arguments memory
void free_command_args(char** argv) {
    if (!argv) return;
    for (int i = 0; argv[i] != NULL; i++) {
        free(argv[i]);
    }
    free(argv);
}

// Compare node names (ignore namespaces)
static int compare_node_names(const xmlChar* name1, const xmlChar* name2) {
    // Skip namespace prefix
    const xmlChar* local_name1 = xmlStrchr(name1, ':');
    const xmlChar* local_name2 = xmlStrchr(name2, ':');
    
    // If no namespace prefix, use full name
    if (local_name1 == NULL) local_name1 = name1;
    else local_name1++; // Skip colon
    if (local_name2 == NULL) local_name2 = name2;
    else local_name2++; // Skip colon
    
    return xmlStrcmp(local_name1, local_name2);
}

// Merge function implementation
int merge_arxml_files(const ProgramOptions *opts) {
    xmlDocPtr base_doc = NULL;
    xmlNodePtr root_node = NULL;
    
    // Parse base file
    base_doc = xmlReadFile(opts->input_files[0], NULL, XML_PARSE_NOBLANKS | XML_PARSE_NOBLANKS | XML_PARSE_COMPACT);
    if (base_doc == NULL) {
        printf("Error: Cannot parse base file '%s'\n", opts->input_files[0]);
        return 0;
    }
    
    // Get root node
    root_node = xmlDocGetRootElement(base_doc);
    if (root_node == NULL) {
        printf("Error: File '%s' is empty\n", opts->input_files[0]);
        xmlFreeDoc(base_doc);
        return 0;
    }
    
    // Process other files
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
        
        // Recursively merge nodes
        xmlNodePtr cur = cur_root->children;
        while (cur != NULL) {
            merge_node(root_node, cur, base_doc);
            cur = cur->next;
        }
        
        xmlFreeDoc(doc);
    }
    
    // 根据缩进样式设置输出格式
    int format_output = 1;  // 启用格式化输出
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
            xmlTreeIndentString = "    "; // 默认使用4空格
    }

    // 设置输出格式
    xmlKeepBlanksDefault(0);
    xmlIndentTreeOutput = 1;

    // 保存文档
    if (xmlSaveFormatFileEnc(opts->output_file, base_doc, "UTF-8", format_output) < 0) {
        printf("Error: Cannot save output file '%s'\n", opts->output_file);
        xmlFreeDoc(base_doc);
        return 0;
    }
    
    xmlFreeDoc(base_doc);
    // Only print completion message if all files were successfully merged
    if (opts->input_file_count > 1) {
        printf("Merge completed, output file: %s\n", opts->output_file);
    }
    return 1;
}

// Check number of SHORT-NAME nodes in node and all its children
static int count_short_names(xmlNodePtr node) {
    // If not an element node, return 0
    if (node->type != XML_ELEMENT_NODE) {
        return 0;
    }
    
    int count = 0;
    xmlNodePtr cur = node->children;
    
    // Recursively check all child nodes
    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"SHORT-NAME"))) {
            count++;
            if (count > 1) {
                return count;  // If multiple found, return immediately
            }
        } else if (cur->type == XML_ELEMENT_NODE) {
            count += count_short_names(cur);
            if (count > 1) {
                return count;  // If multiple found, return immediately
            }
        }
        cur = cur->next;
    }
    
    return count;
}

// Recursively merge nodes
static void merge_node(xmlNodePtr base_parent, xmlNodePtr input_node, xmlDocPtr doc) {
    // Skip text nodes and comment nodes
    if (input_node->type != XML_ELEMENT_NODE) {
        return;
    }

    xmlChar* input_name = get_short_name(input_node);
    xmlNodePtr existing_node = NULL;
    
    // Search for nodes of the same type in base_parent's children
    xmlNodePtr cur = base_parent->children;
    while (cur != NULL) {
        // Compare only nodes of the same type, ignore namespaces
        if (compare_node_names(cur->name, input_node->name) == 0) {
            xmlChar* cur_name = get_short_name(cur);
            
            // If both nodes have SHORT-NAME, compare names
            if (input_name != NULL && cur_name != NULL) {
                if (!xmlStrcmp(cur_name, input_name)) {
                    existing_node = cur;
                }
            } 
            // If neither node has SHORT-NAME, consider them matching
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
        // Found matching node
        if (input_name != NULL && count_short_names(input_node) == 1) {
            // If it's a minimal unit with SHORT-NAME, discard entire subtree
            return;
        }
        // Otherwise process child nodes recursively
        xmlNodePtr input_child = input_node->children;
        while (input_child != NULL) {
            merge_node(existing_node, input_child, doc);
            input_child = input_child->next;
        }
        return;
    }
    
    // No matching node found, copy entire subtree
    xmlNodePtr copy = xmlNewNode(NULL, input_node->name);
    
    xmlNodePtr child = input_node->children;
    while (child != NULL) {
        if (child->type == XML_ELEMENT_NODE) {
            // 递归复制元素节点
            merge_node(copy, child, doc);
        } else if (child->type == XML_TEXT_NODE) {
            xmlChar* content = xmlNodeGetContent(child);
            xmlNodePtr text = xmlNewText(content);
            xmlAddChild(copy, text);
            xmlFree(content);
        }
        child = child->next;
    }
    xmlAddChild(base_parent, copy);
}

// Implement helper function
static xmlChar* get_short_name(xmlNodePtr node) {
    xmlNodePtr cur = node->children;
    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"SHORT-NAME"))) {
            return xmlNodeGetContent(cur);
        }
        cur = cur->next;
    }
    return NULL;
}

// 新增format模式的参数解析函数
int parse_format_options(int argc, char *argv[], ProgramOptions *opts) {
    int opt;
    opts->input_file_count = 0;
    opts->indent_style = INDENT_4SPACE; // 默认使用4空格缩进
    
    // Set default output directory to current directory
    strncpy(opts->output_dir, ".", MAX_PATH - 1);
    
    // Reset getopt
    optind = 0;
    
    while ((opt = getopt(argc, argv, "a:o:i:")) != -1) {
        switch (opt) {
            case 'a':
                if (opts->input_file_count >= MAX_FILES) {
                    printf("Error: Number of input files exceeds limit (%d)\n", MAX_FILES);
                    return 0;
                }
                strncpy(opts->input_files[opts->input_file_count], optarg, MAX_PATH - 1);
                opts->input_file_count++;
                break;
            case 'o':
                strncpy(opts->output_dir, optarg, MAX_PATH - 1);
                break;
            case 'i':
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
                printf("Error: Invalid option or missing argument\n");
                return 0;
            default:
                printf("Error: Unknown option %c\n", opt);
                return 0;
        }
    }
    
    // Check required parameters
    if (opts->input_file_count == 0) {
        printf("Error: Format mode requires at least one input file (-a)\n");
        return 0;
    }
    
    // Check if files exist
    for (int i = 0; i < opts->input_file_count; i++) {
        FILE* file = fopen(opts->input_files[i], "r");
        if (!file) {
            printf("Error: Cannot open input file '%s'\n", opts->input_files[i]);
            return 0;
        }
        fclose(file);
    }
    
    // Check if output directory exists
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

// 新增format功能实现函数
int format_arxml_files(const ProgramOptions *opts) {
    // 先设置全局格式化参数
    xmlKeepBlanksDefault(0);
    xmlIndentTreeOutput = 1;
    int format_output = 1;

    // 根据选项设置缩进字符串
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

    for (int i = 0; i < opts->input_file_count; i++) {
        xmlDocPtr doc = xmlReadFile(opts->input_files[i], NULL, XML_PARSE_NOBLANKS);
        if (doc == NULL) {
            printf("Error: Cannot parse file '%s'\n", opts->input_files[i]);
            return 0;
        }

        // 确定输出路径
        const char* output_path;
        char new_path[MAX_PATH];
        
        if (strcmp(opts->output_dir, ".") == 0) {
            // 如果没有指定输出目录，直接覆盖源文件
            output_path = opts->input_files[i];
        } else {
            // 如果指定了输出目录，构建新的输出路径
            const char* input_filename = strrchr(opts->input_files[i], '/');
            if (input_filename == NULL) {
                input_filename = strrchr(opts->input_files[i], '\\');
            }
            if (input_filename == NULL) {
                input_filename = opts->input_files[i];
            } else {
                input_filename++; // 跳过斜杠
            }
            
            // 安全地构建输出路径
            size_t dir_len = strlen(opts->output_dir);
            size_t file_len = strlen(input_filename);
            
            // 检查路径长度
            if (dir_len + file_len + 2 > MAX_PATH) { // +2 for '/' and '\0'
                printf("Error: Output path too long for file '%s'\n", input_filename);
                xmlFreeDoc(doc);
                return 0;
            }
            
            // 复制目录路径
            strncpy(new_path, opts->output_dir, MAX_PATH - 1);
            new_path[MAX_PATH - 1] = '\0';
            
            // 确保目录路径以斜杠结尾
            if (dir_len > 0 && new_path[dir_len - 1] != '/' && new_path[dir_len - 1] != '\\') {
                if (dir_len + 1 < MAX_PATH) {
                    new_path[dir_len] = '/';
                    new_path[dir_len + 1] = '\0';
                    dir_len++;
                }
            }
            
            // 安全地添加文件名
            if (dir_len + file_len < MAX_PATH) {
                strcpy(new_path + dir_len, input_filename);
                output_path = new_path;
            } else {
                printf("Error: Output path too long\n");
                xmlFreeDoc(doc);
                return 0;
            }
        }

        // 在每个文件处理前重新设置缩进字符串
        xmlTreeIndentString = indent_str;

        // 保存格式化后的文件
        if (xmlSaveFormatFileEnc(output_path, doc, "UTF-8", format_output) < 0) {
            printf("Error: Cannot save formatted file '%s'\n", output_path);
            xmlFreeDoc(doc);
            return 0;
        }

        xmlFreeDoc(doc);
        
        // 根据是否覆盖源文件显示不同的提示信息
        if (strcmp(opts->output_dir, ".") == 0) {
            printf("File formatted in place: %s\n", output_path);
        } else {
            printf("Formatted file saved to: %s\n", output_path);
        }
    }

    return 1;
}

int main(int argc, char *argv[]) {
    // Initialize libxml2
    LIBXML_TEST_VERSION
    
    char** cmd_argv = NULL;
    int cmd_argc = 0;
    int using_cmd_file = 0;

    if (argc < 2) {
        print_usage();
        return 1;
    }

    // 首先解析操作模式
    ProgramOptions opts = {0};
    opts.mode = parse_mode(argv[1]);

    if (opts.mode == MODE_UNKNOWN) {
        printf("Error: Unknown operation mode '%s'\n", argv[1]);
        print_usage();
        return 1;
    }

    // 检查是否使用命令文件
    if (argc == 4 && strcmp(argv[2], "-f") == 0) {
        cmd_argv = read_command_from_file(argv[3], &cmd_argc);
        if (!cmd_argv) {
            return 1;
        }
        // 直接使用从文件读取的参数，不需要添加程序名和模式
        argv = cmd_argv;
        argc = cmd_argc;
        using_cmd_file = 1;
    }

    int success = 0;
    switch (opts.mode) {
        case MODE_MERGE: {
            // Reset getopt before parsing options
            optind = 1;  // 改为从1开始，因为0是程序名
            
            if (using_cmd_file) {
                // 从文件读取的参数直接使用
                success = parse_merge_options(argc, argv, &opts);
            } else {
                // 从命令行输入的参数需要跳过模式名
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
            optind = 1;  // Reset getopt
            
            if (using_cmd_file) {
                // 从文件读取的参数直接使用
                success = parse_format_options(argc, argv, &opts);
            } else {
                // 从命令行输入的参数需要跳过模式名
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

    if (using_cmd_file) {
        free_command_args(cmd_argv);
    }
    
    // Cleanup libxml2
    xmlCleanupParser();
    
    return 0;
} 