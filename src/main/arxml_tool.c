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
    printf("  arXmlTool.exe <mode> -f <command_file>\n");
    printf("  arXmlTool.exe --help\n\n");
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
    printf("                   - If not specified: Keep source file indentation\n");
    printf("                   - 'tab': Use tab for indentation\n");
    printf("                   - '<n>': Use n spaces for indentation (e.g. '2', '4', '8')\n");
    printf("  -i <sort>       Specify indentation style (optional)\n");
    printf("                   - 'asc': Sort in ascending\n");
    printf("                   - 'desc': Sort in descending\n\n");
    printf("Format mode options:\n");
    printf("  -a <file.arxml>  Specify input file (can be used multiple times)\n");
    printf("  -o <directory>   Specify output directory (optional, will overwrite source files if not specified)\n");
    printf("  -i <style>       Specify indentation style (optional)\n");
    printf("                   - If not specified: Keep source file indentation\n");
    printf("                   - 'tab': Use tab for indentation\n");
    printf("                   - '<n>': Use n spaces for indentation (e.g. '2', '4', '8')\n");
    printf("  -i <sort>       Specify indentation style (optional)\n");
    printf("                   - 'asc': Sort in ascending\n");
    printf("                   - 'desc': Sort in descending\n");
}

/* Program entry point | 程序入口点 */
int main(int argc, char *argv[]) {
    ProgramOptions opts;
    char **cmd_argv = NULL;
    int cmd_argc = 0;
    int result = 0;

    /* Initialize options | 初始化选项 */
    memset(&opts, 0, sizeof(ProgramOptions));
    opts.mode = MODE_UNKNOWN;
    opts.indent_style = INDENT_DEFAULT;
    opts.indent_width = 4;  /* Default to 4 spaces | 默认使用4空格缩进 */
    opts.sort_order = SORT_NONE;
    strncpy(opts.output_dir, ".", MAX_PATH - 1);

    /* Process command file if specified | 如果指定了命令文件则处理 */
    if (argc == 4 && strcmp(argv[2], "-f") == 0) {
        cmd_argv = read_command_from_file(argv[3], &cmd_argc);
        if (!cmd_argv) {
            printf("Error: Cannot read command file '%s'\n", argv[3]);
            return 1;
        }
        
        /* Parse options from command file | 从命令文件解析选项 */
        opts.mode = parse_mode(argv[1]);
        if (opts.mode == MODE_MERGE) {
            if (!parse_merge_options(cmd_argc, cmd_argv, &opts)) {
                free_command_args(cmd_argv);
                return 1;
            }
        } else if (opts.mode == MODE_FORMAT) {
            if (!parse_format_options(cmd_argc, cmd_argv, &opts)) {
                free_command_args(cmd_argv);
                return 1;
            }
        } else {
            printf("Error: Invalid operation mode\n");
            free_command_args(cmd_argv);
            return 1;
        }
    } else {
        /* Parse options from command line | 从命令行解析选项 */
        if (!parse_options(argc, argv, &opts)) {
            return 1;
        }
    }

    /* Process according to operation mode | 根据操作模式处理 */
    switch (opts.mode) {
        case MODE_MERGE:
            result = merge_arxml_files(&opts);
            break;
        case MODE_FORMAT:
            result = format_arxml_files(&opts);
            break;
        default:
            printf("Error: Invalid operation mode\n");
            result = 1;
            break;
    }

    /* Free command file arguments after processing | 处理完成后释放命令文件参数 */
    if (cmd_argv) {
        free_command_args(cmd_argv);
    }

    return result;
} 