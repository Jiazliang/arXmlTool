#include "options.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include "../utils/fs_utils.h"
#include "../main/arxml_tool.h"

#ifdef _WIN32
#include <io.h>
#define access _access
#else
#include <unistd.h>
#endif

extern int optind;  /* 声明 optind */

/* Parse command line options | 解析命令行选项 */
int parse_options(int argc, char *argv[], ProgramOptions *opts) {
    /* Reset getopt | 重置getopt */
    optind = 1;
    
    if (argc < 2) {
        printf("Error: No operation mode specified\n");
        return 0;
    }

    /* Initialize options | 初始化选项 */
    memset(opts, 0, sizeof(ProgramOptions));
    opts->mode = MODE_UNKNOWN;
    opts->indent_style = INDENT_DEFAULT;
    opts->sort_order = SORT_NONE;
    strncpy(opts->output_dir, ".", MAX_PATH - 1);

    /* Parse operation mode | 解析操作模式 */
    if (strcmp(argv[1], "merge") == 0) {
        opts->mode = MODE_MERGE;
        return parse_merge_options(argc - 1, argv + 1, opts);
    } else if (strcmp(argv[1], "format") == 0) {
        opts->mode = MODE_FORMAT;
        return parse_format_options(argc - 1, argv + 1, opts);
    } else if (strcmp(argv[1], "--help") == 0) {
        print_usage();
        return 0;
    } else {
        printf("Error: Unknown operation mode '%s'\n", argv[1]);
        return 0;
    }
}

/* Parse merge mode options | 解析合并模式的选项 */
int parse_merge_options(int argc, char *argv[], ProgramOptions *opts) {
    int opt;
    opts->input_file_count = 0;
    
    /* Reset getopt | 重置getopt */
    optind = 1;
    
    while ((opt = getopt(argc, argv, "a:m:o:i:s:")) != -1) {
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
                } else {
                    /* Try to parse number of spaces | 尝试解析空格数 */
                    char* endptr;
                    long spaces = strtol(optarg, &endptr, 10);
                    if (*endptr != '\0' || spaces <= 0) {
                        printf("Error: Invalid indent style '%s'. Use 'tab' or a positive number\n", optarg);
                        return 0;
                    }
                    opts->indent_style = INDENT_SPACE;
                    opts->indent_width = (int)spaces;
                }
                break;
                case 's':
                if (strcmp(optarg, "asc") == 0) {
                    opts->sort_order = SORT_ASC;
                } else if (strcmp(optarg, "desc") == 0) {
                    opts->sort_order = SORT_DESC;
                } else {
                    printf("Error: Invalid sort order '%s'. Use 'asc' or 'desc'\n", optarg);
                    return 0;
                }
                break;
            case '?':
                printf("Error: Invalid option or missing argument\n");
                return 0;
        }
    }

    if (opts->input_file_count == 0 || opts->output_file[0] == '\0') {
        printf("Error: Merge mode requires at least one input file (-a) and one output file (-m)\n");
        return 0;
    }

    return 1;
}

/* Parse format mode options | 解析格式化模式的选项 */
int parse_format_options(int argc, char *argv[], ProgramOptions *opts) {
    int opt;
    opts->input_file_count = 0;
    
    /* Reset getopt | 重置getopt */
    optind = 1;
    
    while ((opt = getopt(argc, argv, "a:o:i:s:")) != -1) {
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
                } else {
                    /* Try to parse number of spaces | 尝试解析空格数 */
                    char* endptr;
                    long spaces = strtol(optarg, &endptr, 10);
                    if (*endptr != '\0' || spaces <= 0) {
                        printf("Error: Invalid indent style '%s'. Use 'tab' or a positive number\n", optarg);
                        return 0;
                    }
                    opts->indent_style = INDENT_SPACE;
                    opts->indent_width = (int)spaces;
                }
                break;
            case 's':
                if (strcmp(optarg, "asc") == 0) {
                    opts->sort_order = SORT_ASC;
                } else if (strcmp(optarg, "desc") == 0) {
                    opts->sort_order = SORT_DESC;
                } else {
                    printf("Error: Invalid sort order '%s'. Use 'asc' or 'desc'\n", optarg);
                    return 0;
                }
                break;
            case '?':
                printf("Error: Invalid option or missing argument\n");
                return 0;
        }
    }

    if (opts->input_file_count == 0) {
        printf("Error: Format mode requires at least one input file (-a)\n");
        return 0;
    }

    return 1;
} 