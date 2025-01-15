#include "options.h"
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "../utils/fs_utils.h"

#ifdef _WIN32
#include <io.h>
#define access _access
#else
#include <unistd.h>
#endif

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
    
    /* Create output directory if it doesn't exist | 如果输出目录不存在则创建 */
    if (strcmp(opts->output_dir, ".") != 0) {
        if (!create_directories(opts->output_dir)) {
            printf("Error: Cannot create output directory '%s'\n", opts->output_dir);
            return 0;
        }
    }
    
    return 1;
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
    
    /* Create output directory if it doesn't exist | 如果输出目录不存在则创建 */
    if (strcmp(opts->output_dir, ".") != 0) {
        if (!create_directories(opts->output_dir)) {
            printf("Error: Cannot create output directory '%s'\n", opts->output_dir);
            return 0;
        }
    }
    
    return 1;
} 