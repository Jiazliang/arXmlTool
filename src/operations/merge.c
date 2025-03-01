#include "merge.h"
#include "../utils/xml_utils.h"
#include "../utils/fs_utils.h"  /* 添加头文件引用 */
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

/* Recursively merge nodes | 递归合并节点 */
static void merge_node(xmlNodePtr base_parent, xmlNodePtr input_node, xmlDocPtr doc) {
    /* Skip text nodes and comment nodes | 跳过文本节点和注释节点 */
    if (input_node->type != XML_ELEMENT_NODE) {
        return;
    }

    xmlChar* input_name = get_short_name(input_node);
    xmlNodePtr existing_node = NULL;
    
    /* Search for nodes of the same type in base_parent's children | 在基础父节点的子节点中搜索相同类型的节点 */
    xmlNodePtr cur = base_parent->children;
    while (cur != NULL) {
        /* Compare only nodes of the same type, ignore namespaces | 只比较相同类型的节点，忽略命名空间 */
        if (compare_node_names(cur->name, input_node->name) == 0) {
            xmlChar* cur_name = get_short_name(cur);
            
            /* If both nodes have SHORT-NAME, compare names | 如果两个节点都有SHORT-NAME，比较名称 */
            if (input_name != NULL && cur_name != NULL) {
                if (!xmlStrcmp(cur_name, input_name)) {
                    existing_node = cur;
                }
            } 
            /* If neither node has SHORT-NAME, consider them matching | 如果两个节点都没有SHORT-NAME，认为它们匹配 */
            else if (input_name == NULL && cur_name == NULL) {
                existing_node = cur;
            }
            
            if (cur_name != NULL) {
                xmlFree(cur_name);
            }
            
            if (existing_node) {
                break;
            }
        }
        cur = cur->next;
    }
    
    if (input_name != NULL) {
        xmlFree(input_name);
    }
    
    if (existing_node != NULL) {
        /* Found matching node | 找到匹配的节点 */
        if (input_name != NULL && count_short_names(input_node) == 1) {
            /* If it's a minimal unit with SHORT-NAME, discard entire subtree | 如果是带有SHORT-NAME的最小单元，丢弃整个子树 */
            return;
        }
        /* Otherwise process child nodes recursively | 否则递归处理子节点 */
        xmlNodePtr input_child = input_node->children;
        while (input_child != NULL) {
            merge_node(existing_node, input_child, doc);
            input_child = input_child->next;
        }
        return;
    }
    
    /* No matching node found, copy entire subtree | 未找到匹配节点，复制整个子树 */
    xmlNodePtr copy = xmlNewNode(NULL, input_node->name);
    
    xmlNodePtr child = input_node->children;
    while (child != NULL) {
        if (child->type == XML_ELEMENT_NODE) {
            /* Recursively copy element nodes | 递归复制元素节点 */
            merge_node(copy, child, doc);
        } else if (child->type == XML_TEXT_NODE) {
            /* Copy text content | 复制文本内容 */
            xmlChar* content = xmlNodeGetContent(child);
            xmlNodePtr text = xmlNewText(content);
            xmlAddChild(copy, text);
            xmlFree(content);
        }
        child = child->next;
    }
    xmlAddChild(base_parent, copy);
}

/* Get final output path based on options | 根据选项获取最终输出路径 */
static void get_final_output_path(const ProgramOptions *opts, char *final_path, size_t size) {
    if (strcmp(opts->output_dir, ".") == 0) {
        /* No -o parameter, use path from -m directly | 没有-o参数，直接使用-m的路径 */
        strncpy(final_path, opts->output_file, size);
        final_path[size - 1] = '\0';
    } else {
        /* -o parameter exists, combine output_dir with filename from -m | 存在-o参数，将output_dir与-m的文件名组合 */
        char filename[MAX_PATH];
        const char *last_slash = strrchr(opts->output_file, '/');
        const char *last_backslash = strrchr(opts->output_file, '\\');
        const char *last_separator = last_slash > last_backslash ? last_slash : last_backslash;
        
        /* Get filename from output_file | 从output_file中获取文件名 */
        if (last_separator) {
            strncpy(filename, last_separator + 1, sizeof(filename));
        } else {
            strncpy(filename, opts->output_file, sizeof(filename));
        }
        filename[sizeof(filename) - 1] = '\0';
        
        /* Combine output_dir with filename | 组合output_dir和文件名 */
        snprintf(final_path, size, "%s/%s", opts->output_dir, filename);
    }
}

/* Remove the first comment node of the document | 移除文档的第一个注释节点 */
static void remove_first_comment(xmlDocPtr doc) {
    /* Get the first child node of the document | 获取文档的第一个子节点 */
    xmlNodePtr node = doc->children;
    while (node != NULL) {
        /* Check if the node is a comment node | 检查是否为注释节点 */
        if (node->type == XML_COMMENT_NODE) {
            /* Remove the node from the document | 从文档中移除节点 */
            xmlUnlinkNode(node);
            /* Free the node memory | 释放节点内存 */
            xmlFreeNode(node);
            /* Exit loop after removing the first comment | 只删除第一个注释，找到后退出循环 */
            break;
        }
        /* Move to the next node | 继续检查下一个节点 */
        node = node->next;
    }
}

/* Merge ARXML files implementation | ARXML文件合并实现 */
int merge_arxml_files(const ProgramOptions *opts) {
    xmlDocPtr base_doc = NULL;
    xmlNodePtr root_node = NULL;
    
    /* Initialize detected indentation style | 初始化检测到的缩进风格 */
    DetectedIndentStyle detected = {.style = 's', .width = 4};  /* Default to 4 spaces | 默认4空格 */
    if (opts->indent_style == INDENT_DEFAULT) {
        detected = detect_indent_style(opts->input_files[0]);
    }

    /* Parse base file | 解析基础文件 */
    base_doc = xmlReadFile(opts->input_files[0], NULL, XML_PARSE_NOBLANKS | XML_PARSE_NOBLANKS | XML_PARSE_COMPACT);
    if (base_doc == NULL) {
        printf("Error: Cannot parse base file '%s'\n", opts->input_files[0]);
        return 0;
    }

    /* Remove the first comment node of the document | 移除文档的第一个注释节点 */
    remove_first_comment(base_doc);
    
    /* Get root node | 获取根节点 */
    root_node = xmlDocGetRootElement(base_doc);
    if (root_node == NULL) {
        printf("Error: File '%s' is empty\n", opts->input_files[0]);
        xmlFreeDoc(base_doc);
        return 0;
    }
    
    /* Process other files | 处理其他文件 */
    for (int i = 1; i < opts->input_file_count; i++) {
        xmlDocPtr doc = xmlReadFile(opts->input_files[i], NULL, XML_PARSE_NOBLANKS);
        if (doc == NULL) {
            printf("Error: Cannot parse file '%s'\n", opts->input_files[i]);
            xmlFreeDoc(base_doc);
            return 0;
        }
        
        xmlNodePtr cur_root = xmlDocGetRootElement(doc);
        if (cur_root == NULL) {
            printf("Error: File '%s' is empty\n", opts->input_files[i]);
            xmlFreeDoc(doc);
            xmlFreeDoc(base_doc);
            return 0;
        }
        
        /* Recursively merge nodes | 递归合并节点 */
        xmlNodePtr cur = cur_root->children;
        while (cur != NULL) {
            merge_node(root_node, cur, base_doc);
            cur = cur->next;
        }
        
        xmlFreeDoc(doc);
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
    } else if (opts->indent_style == INDENT_SPACE) {
        /* Use specified number of spaces | 使用指定数量的空格 */
        xmlTreeIndentString = create_space_indent(opts->indent_width);
    }

    /* Get final output path | 获取最终输出路径 */
    char final_output_path[MAX_PATH];
    get_final_output_path(opts, final_output_path, sizeof(final_output_path));

    /* Create output directory if needed | 如果需要则创建输出目录 */
    char output_dir[MAX_PATH];
    get_directory_path(final_output_path, output_dir, sizeof(output_dir));
    if (!create_directories(output_dir)) {
        printf("Error: Cannot create output directory for file '%s'\n", final_output_path);
        xmlFreeDoc(base_doc);
        return 0;
    }

    /* Sort nodes if requested | 如果需要则进行排序 */
    if (opts->sort_order != SORT_NONE) {
        xmlNodePtr root = xmlDocGetRootElement(base_doc);
        if (!root) {
            printf("Error: Empty document\n");
            xmlFreeDoc(base_doc);
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

    /* Save the merged document | 保存合并后的文档 */
    if (xmlSaveFormatFileEnc(final_output_path, base_doc, "UTF-8", 1) < 0) {
        printf("Error: Cannot save file '%s'\n", final_output_path);
        xmlFreeDoc(base_doc);
        return 0;
    }
    
    xmlFreeDoc(base_doc);
    /* Print completion message | 打印完成消息 */
    if (opts->input_file_count > 1) {
        printf("Merge completed, output file: %s\n", final_output_path);
    }
    return 1;
}