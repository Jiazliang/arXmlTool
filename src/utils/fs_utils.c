#include "fs_utils.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <stddef.h>  /* For size_t | 用于size_t类型 */

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#endif

/* Get directory path from file path | 从文件路径中获取目录路径 */
void get_directory_path(const char* file_path, char* dir_path, size_t size) {
    strncpy(dir_path, file_path, size);
    dir_path[size - 1] = '\0';
    
    char* last_slash = strrchr(dir_path, '/');
    char* last_backslash = strrchr(dir_path, '\\');
    
    /* Use the rightmost slash or backslash | 使用最右边的斜杠或反斜杠 */
    char* last_separator = last_slash > last_backslash ? last_slash : last_backslash;
    
    if (last_separator) {
        /* Terminate string at separator | 在分隔符处截断字符串 */
        *(last_separator) = '\0';
    } else {
        /* No directory part, use current directory | 没有目录部分，使用当前目录 */
        strncpy(dir_path, ".", size);
    }
}

/* Create directory recursively | 递归创建目录 */
int create_directories(const char* path) {
    if (path == NULL || strlen(path) == 0 || strcmp(path, ".") == 0) {
        return 1;  /* Current directory always exists | 当前目录总是存在的 */
    }

    char tmp[256];
    char* p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    
    /* Remove trailing slash | 移除末尾的斜杠 */
    if (tmp[len - 1] == '/' || tmp[len - 1] == '\\') {
        tmp[len - 1] = 0;
    }
    
    /* Create parent directories | 创建父目录 */
    for (p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = 0;
            #ifdef _WIN32
            if (_mkdir(tmp) != 0 && errno != EEXIST) {
            #else
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
            #endif
                return 0;
            }
            *p = '/';
        }
    }
    
    /* Create the final directory | 创建最终目录 */
    #ifdef _WIN32
    if (_mkdir(tmp) != 0 && errno != EEXIST) {
    #else
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
    #endif
        return 0;
    }
    
    return 1;
} 