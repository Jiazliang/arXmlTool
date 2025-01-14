#ifndef COMMAND_H
#define COMMAND_H

/* Read command line from file | 从文件读取命令行参数 */
char** read_command_from_file(const char* filename, int* argc);

/* Free command line arguments memory | 释放命令行参数内存 */
void free_command_args(char** argv);

#endif /* COMMAND_H */ 