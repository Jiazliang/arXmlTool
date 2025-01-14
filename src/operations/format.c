#include "format.h"
#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>

/* Format ARXML files implementation | ARXML文件格式化实现 */
int format_arxml_files(const ProgramOptions *opts) {
    /* Set global formatting parameters | 设置全局格式化参数 */
    xmlKeepBlanksDefault(0);
    xmlIndentTreeOutput = 1;
    int format_output = 1;

    /* Set indentation string based on options | 根据选项设置缩进字符串 */
    const char* indent_str;
    switch (opts->indent_style) {
        case INDENT_TAB:
            indent_str = "\t";
            break;
        case INDENT_2SPACE:
            indent_str = "  ";
            break;
        case INDENT_4SPACE:
            indent_str = "    ";
            break;
        default:
            indent_str = "    ";
    }

    /* Process each input file | 处理每个输入文件 */
    for (int i = 0; i < opts->input_file_count; i++) {
        /* Parse XML file | 解析XML文件 */
        xmlDocPtr doc = xmlReadFile(opts->input_files[i], NULL, XML_PARSE_NOBLANKS);
        if (doc == NULL) {
            printf("Error: Cannot parse file '%s'\n", opts->input_files[i]);
            return 0;
        }

        /* Determine output path | 确定输出路径 */
        const char* output_path;
        char new_path[MAX_PATH];
        
        if (strcmp(opts->output_dir, ".") == 0) {
            /* If no output directory specified, overwrite source file | 如果没有指定输出目录，直接覆盖源文件 */
            output_path = opts->input_files[i];
        } else {
            /* Extract filename from input path | 从输入路径中提取文件名 */
            const char* input_filename = strrchr(opts->input_files[i], '/');
            if (input_filename == NULL) {
                input_filename = strrchr(opts->input_files[i], '\\');
            }
            if (input_filename == NULL) {
                input_filename = opts->input_files[i];
            } else {
                input_filename++; /* Skip slash | 跳过斜杠 */
            }
            
            /* Safely construct output path | 安全地构建输出路径 */
            size_t dir_len = strlen(opts->output_dir);
            size_t file_len = strlen(input_filename);
            
            /* Check path length | 检查路径长度 */
            if (dir_len + file_len + 2 > MAX_PATH) { /* +2 for '/' and '\0' */
                printf("Error: Output path too long for file '%s'\n", input_filename);
                xmlFreeDoc(doc);
                return 0;
            }
            
            /* Copy directory path | 复制目录路径 */
            strncpy(new_path, opts->output_dir, MAX_PATH - 1);
            new_path[MAX_PATH - 1] = '\0';
            
            /* Ensure directory path ends with slash | 确保目录路径以斜杠结尾 */
            if (dir_len > 0 && new_path[dir_len - 1] != '/' && new_path[dir_len - 1] != '\\') {
                if (dir_len + 1 < MAX_PATH) {
                    new_path[dir_len] = '/';
                    new_path[dir_len + 1] = '\0';
                    dir_len++;
                }
            }
            
            /* Safely append filename | 安全地添加文件名 */
            if (dir_len + file_len < MAX_PATH) {
                strcpy(new_path + dir_len, input_filename);
                output_path = new_path;
            } else {
                printf("Error: Output path too long\n");
                xmlFreeDoc(doc);
                return 0;
            }
        }

        /* Set indentation string before processing each file | 在处理每个文件前设置缩进字符串 */
        xmlTreeIndentString = indent_str;

        /* Save formatted file | 保存格式化后的文件 */
        if (xmlSaveFormatFileEnc(output_path, doc, "UTF-8", format_output) < 0) {
            printf("Error: Cannot save formatted file '%s'\n", output_path);
            xmlFreeDoc(doc);
            return 0;
        }

        xmlFreeDoc(doc);
        
        /* Print appropriate message based on output location | 根据输出位置打印相应的消息 */
        if (strcmp(opts->output_dir, ".") == 0) {
            printf("File formatted in place: %s\n", output_path);
        } else {
            printf("Formatted file saved to: %s\n", output_path);
        }
    }

    return 1;
} 