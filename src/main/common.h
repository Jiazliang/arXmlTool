#ifndef COMMON_H
#define COMMON_H

/* Constants definition | 常量定义 */
#define MAX_FILES 1024
#define MAX_PATH 256
#define MAX_CMD_LENGTH 4096
#define MAX_BUFFER_SIZE 8192

/* Operation modes enumeration | 操作模式枚举 */
typedef enum {
    MODE_MERGE,    /* Merge mode | 合并模式 */
    MODE_COMPARE,  /* Compare mode | 比较模式 */
    MODE_GENERATE, /* Generate mode | 生成模式 */
    MODE_FORMAT,   /* Format mode | 格式化模式 */
    MODE_UNKNOWN   /* Unknown mode | 未知模式 */
} OperationMode;

/* Program options structure | 程序选项结构 */
typedef struct {
    OperationMode mode;
    char input_files[MAX_FILES][MAX_PATH];
    int input_file_count;
    char output_file[MAX_PATH];
    char output_dir[MAX_PATH];
    enum {
        INDENT_TAB,    /* Tab indentation | Tab缩进 */
        INDENT_2SPACE, /* 2 spaces indentation | 2空格缩进 */
        INDENT_4SPACE  /* 4 spaces indentation | 4空格缩进 */
    } indent_style;
} ProgramOptions;

#endif /* COMMON_H */ 