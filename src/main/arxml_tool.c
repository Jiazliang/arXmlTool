/* Use static library | 使用静态库 */
#ifndef LIBXML_STATIC
#define LIBXML_STATIC
#endif

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "arxml_tool.h"
#include "options.h"

/* Parse operation mode from string | 从字符串解析操作模式 */
OperationMode parse_mode(const char* mode_str) {
    if (strcmp(mode_str, "merge") == 0) return MODE_MERGE;
    if (strcmp(mode_str, "compare") == 0) return MODE_COMPARE;
    if (strcmp(mode_str, "generate") == 0) return MODE_GENERATE;
    if (strcmp(mode_str, "format") == 0) return MODE_FORMAT;
    return MODE_UNKNOWN;
}

/* Print usage information | 打印使用说明 */
void print_usage(void) {
    printf("Usage:\n");
    printf("  arXmlTool.exe <mode> [options]\n");
    printf("  arXmlTool.exe <mode> -f <command_file>\n\n");
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