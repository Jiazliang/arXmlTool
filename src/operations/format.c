#include "format.h"
#include "../utils/xml_utils.h"
#include "../utils/fs_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/parser.h>

/* Create a string with n spaces for indentation | 创建包含n个空格的缩进字符串 */
static const char* create_space_indent(int n) {
    static char indent_buf[32] = {0};  /* 足够容纳大量空格 */
    if (n <= 0) n = 4;  /* 如果指定无效数值，使用4空格 */
    if (n > 31) n = 31; /* 限制最大空格数 */
    
    memset(indent_buf, ' ', n);
    indent_buf[n] = '\0';
    return indent_buf;
}

int format_arxml_files(const ProgramOptions *opts) {
    for (int i = 0; i < opts->input_file_count; i++) {
        /* Get output file path | 获取输出文件路径 */
        char output_path[MAX_PATH];
        if (strcmp(opts->output_dir, ".") == 0) {
            strncpy(output_path, opts->input_files[i], MAX_PATH - 1);
            output_path[MAX_PATH - 1] = '\0';
        } else {
            const char *input_filename = strrchr(opts->input_files[i], '/');
            if (!input_filename) input_filename = opts->input_files[i];
            else input_filename++; // Skip '/'
            
            size_t dir_len = strlen(opts->output_dir);
            size_t file_len = strlen(input_filename);
            
            if (dir_len + file_len + 2 > MAX_PATH) {  // +2 for '/' and '\0'
                printf("Error: Output path too long\n");
                return 0;
            }
            
            strcpy(output_path, opts->output_dir);
            output_path[dir_len] = '/';
            strcpy(output_path + dir_len + 1, input_filename);
        }

        /* First read: detect indentation if needed | 第一次读取：如果需要则检测缩进 */
        DetectedIndentStyle detected = {.style = 's', .width = 4};  /* Default to 4 spaces | 默认4空格 */
        if (opts->indent_style == INDENT_DEFAULT) {
            detected = detect_indent_style(opts->input_files[i]);
        }

        /* Second read: process content | 第二次读取：处理内容 */
        xmlDocPtr doc = xmlReadFile(opts->input_files[i], NULL, XML_PARSE_NOBLANKS);
        if (!doc) {
            printf("Error: Cannot parse file '%s'\n", opts->input_files[i]);
            return 0;
        }

        /* Sort nodes if requested | 如果需要则进行排序 */
        if (opts->sort_order != SORT_NONE) {
            xmlNodePtr root = xmlDocGetRootElement(doc);
            if (!root) {
                printf("Error: Empty document\n");
                xmlFreeDoc(doc);
                return 0;
            }
            
            if (opts->sort_specific_tag) {
                /* Sort children of specific tag | 对特定标签的子节点进行排序 */
                int sorted_count = sort_specific_tag_children(root, opts->target_tag, opts->sort_order);
                if (sorted_count == 0) {
                    printf("Warning: No matching tags found for '%s'\n", opts->target_tag);
                }
            } else {
                /* Sort all nodes recursively | 递归排序所有节点 */
                sort_nodes_by_short_name(root, opts->sort_order);
            }
        }

        /* Set indentation for output | 设置输出的缩进 */
        xmlKeepBlanksDefault(0);
        xmlIndentTreeOutput = 1;

        if (opts->indent_style == INDENT_DEFAULT) {
            /* Use detected indentation | 使用检测到的缩进 */
            xmlTreeIndentString = (detected.style == 't') ? "\t" : 
                                create_space_indent(detected.width);
        } else if (opts->indent_style == INDENT_TAB) {
            /* Use tab indentation | 使用制表符缩进 */
            xmlTreeIndentString = "\t";
        } else {
            /* Use specified number of spaces | 使用指定数量的空格 */
            xmlTreeIndentString = create_space_indent(opts->indent_width);
        }

        /* Create output directory if needed | 如果需要则创建输出目录 */
        char output_dir[MAX_PATH];
        get_directory_path(output_path, output_dir, sizeof(output_dir));
        if (!create_directories(output_dir)) {
            printf("Error: Cannot create output directory for file '%s'\n", output_path);
            xmlFreeDoc(doc);
            return 0;
        }

        /* Save the document | 保存文档 */
        if (xmlSaveFormatFileEnc(output_path, doc, "UTF-8", 1) < 0) {
            printf("Error: Cannot save file '%s'\n", output_path);
            xmlFreeDoc(doc);
            return 0;
        }

        xmlFreeDoc(doc);
        printf("File formatted: %s\n", output_path);
    }

    return 1;
}