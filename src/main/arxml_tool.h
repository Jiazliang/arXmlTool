#ifndef ARXML_TOOL_H
#define ARXML_TOOL_H

#include <libxml/parser.h>
#include "common.h"
#include "../command/command.h"
#include "../operations/merge.h"
#include "../operations/format.h"

/* Parse operation mode from string | 从字符串解析操作模式 */
OperationMode parse_mode(const char* mode_str);

/* Print usage information | 打印使用说明 */
void print_usage(void);

#endif /* ARXML_TOOL_H */ 