#ifndef COMMAND_H
#define COMMAND_H

#include "../main/common.h"

#define MAX_BUFFER_SIZE (MAX_PATH * MAX_FILES)
#define MAX_CMD_LENGTH MAX_PATH

/* Read commands from file | 从文件读取命令 */
char** read_command_from_file(const char* filename, int* argc);

/* Free command arguments | 释放命令参数内存 */
void free_command_args(char** argv);

#endif /* COMMAND_H */ 