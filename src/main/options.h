#ifndef OPTIONS_H
#define OPTIONS_H

#include "common.h"

/* Parse operation mode | 解析操作模式 */
OperationMode parse_mode(const char* mode_str);

/* Parse command line options | 解析命令行选项 */
int parse_options(int argc, char *argv[], ProgramOptions *opts);

/* Parse merge mode options | 解析合并模式的选项 */
int parse_merge_options(int argc, char *argv[], ProgramOptions *opts);

/* Parse format mode options | 解析格式化模式的选项 */
int parse_format_options(int argc, char *argv[], ProgramOptions *opts);

#endif /* OPTIONS_H */ 