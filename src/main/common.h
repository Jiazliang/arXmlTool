#ifndef COMMON_H
#define COMMON_H

/* 在文件开头添加版本信息常量 */
#define ARXML_TOOL_VERSION "1.1.0"

#define MAX_PATH 256
#define MAX_FILES 1024

/* Operation mode | 操作模式 */
typedef enum {
    MODE_UNKNOWN,
    MODE_MERGE,
    MODE_FORMAT,
    MODE_COMPARE,
    MODE_GENERATE
} OperationMode;

/* Sort order for format/merge operations | 格式化/合并操作的排序方式 */
typedef enum {
    SORT_NONE = -1,  /* No sorting | 不排序 */
    SORT_ASC = 0,    /* Ascending order | 升序 */
    SORT_DESC = 1    /* Descending order | 降序 */
} SortOrder;

/* Indent style for format/merge operations | 格式化/合并操作的缩进风格 */
typedef enum {
    INDENT_DEFAULT = -1,  /* Use source file's style | 使用源文件的风格 */
    INDENT_TAB = 0,      /* Tab indentation | Tab缩进 */
    INDENT_SPACE = 1     /* Space indentation | 空格缩进 */
} IndentStyle;

/* Program options | 程序选项 */
typedef struct {
    OperationMode mode;
    char input_files[MAX_FILES][MAX_PATH];
    int input_file_count;
    char output_file[MAX_PATH];
    char output_dir[MAX_PATH];
    IndentStyle indent_style;
    int indent_width;        /* Number of spaces for indentation | 缩进的空格数 */
    SortOrder sort_order;
    char target_tag[256];    /* Target tag name for sorting | 要排序的目标标签名 */
    int sort_specific_tag;   /* Whether to sort specific tag only | 是否只对特定标签排序 */
} ProgramOptions;

#endif /* COMMON_H */