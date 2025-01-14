#ifndef OPTIONS_H
#define OPTIONS_H

#include "common.h"

/* Parse merge mode options | 解析合并模式的选项 */
int parse_merge_options(int argc, char *argv[], ProgramOptions *opts);

/* Parse format mode options | 解析格式化模式的选项 */
int parse_format_options(int argc, char *argv[], ProgramOptions *opts);

#endif /* OPTIONS_H */ 