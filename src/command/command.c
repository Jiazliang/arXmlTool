#include "command.h"
#include "../main/arxml_tool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Read command line from file | 从文件读取命令行参数 */
char** read_command_from_file(const char* filename, int* argc) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open command file '%s'\n", filename);
        return NULL;
    }

    /* Read all lines and merge them into one line | 读取所有行并合并成一行 */
    char buffer[MAX_BUFFER_SIZE] = {0};
    char line[MAX_CMD_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        /* Remove trailing newline | 移除行尾的换行符 */
        line[strcspn(line, "\r\n")] = 0;
        
        /* Skip empty lines | 跳过空行 */
        if (strlen(line) == 0) {
            continue;
        }
        
        /* Add space as separator if buffer is not empty | 如果buffer不为空，添加空格作为分隔符 */
        if (strlen(buffer) > 0) {
            strcat(buffer, " ");
        }
        
        /* Check if buffer has enough space | 检查buffer是否有足够空间 */
        if (strlen(buffer) + strlen(line) >= MAX_BUFFER_SIZE - 1) {
            printf("Error: Command line too long\n");
            fclose(file);
            return NULL;
        }
        
        /* Append current line to buffer | 添加当前行到buffer */
        strcat(buffer, line);
    }
    fclose(file);

    if (strlen(buffer) == 0) {
        printf("Error: Command file is empty\n");
        return NULL;
    }

    /* Count arguments | 计算参数个数 */
    int count = 1; /* Program name takes first position | 程序名占用第一个位置 */
    char* p = buffer;
    while (*p) {
        if (*p == ' ' || *p == '\t') {
            count++;
            while (*p && (*p == ' ' || *p == '\t')) p++;
        } else {
            p++;
        }
    }

    /* Allocate memory for argument array | 为参数数组分配内存 */
    char** argv = (char**)malloc((count + 1) * sizeof(char*));
    if (!argv) {
        printf("Error: Memory allocation failed\n");
        return NULL;
    }

    /* Set program name as first argument | 设置程序名为第一个参数 */
    argv[0] = strdup("arXmlTool.exe");

    /* Split command line into arguments | 将命令行分割为参数 */
    int i = 1;
    char* token = strtok(buffer, " \t");
    while (token) {
        argv[i] = strdup(token);
        token = strtok(NULL, " \t");
        i++;
    }
    argv[i] = NULL;
    *argc = i;

    return argv;
}

/* Free command line arguments memory | 释放命令行参数内存 */
void free_command_args(char** argv) {
    if (!argv) return;
    for (int i = 0; argv[i] != NULL; i++) {
        free(argv[i]);
    }
    free(argv);
} 