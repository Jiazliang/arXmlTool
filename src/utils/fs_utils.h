#ifndef FS_UTILS_H
#define FS_UTILS_H

#include <stddef.h>

/* Create directory recursively | 递归创建目录 */
int create_directories(const char* path);

/* Get directory path from file path | 从文件路径中获取目录路径 */
void get_directory_path(const char* file_path, char* dir_path, size_t size);

#endif /* FS_UTILS_H */ 