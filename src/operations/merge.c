#include "merge.h"
#include "../utils/xml_utils.h"
#include <stdio.h>
#include <string.h>

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

/* Merge ARXML files implementation | ARXML文件合并实现 */
int merge_arxml_files(const ProgramOptions *opts) {
    xmlDocPtr base_doc = NULL;
    xmlNodePtr root_node = NULL;
    
    /* Parse base file | 解析基础文件 */
    base_doc = xmlReadFile(opts->input_files[0], NULL, XML_PARSE_NOBLANKS | XML_PARSE_NOBLANKS | XML_PARSE_COMPACT);
    if (base_doc == NULL) {
        printf("Error: Cannot parse base file '%s'\n", opts->input_files[0]);
        return 0;
    }
    
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
    
    /* Set output format based on indentation style | 根据缩进样式设置输出格式 */
    int format_output = 1;  /* Enable formatted output | 启用格式化输出 */
    switch (opts->indent_style) {
        case INDENT_TAB:
            xmlTreeIndentString = "\t";
            break;
        case INDENT_2SPACE:
            xmlTreeIndentString = "  ";
            break;
        case INDENT_4SPACE:
            xmlTreeIndentString = "    ";
            break;
        default:
            xmlTreeIndentString = "    "; /* Default to 4 spaces | 默认使用4空格 */
    }

    /* Configure output settings | 配置输出设置 */
    xmlKeepBlanksDefault(0);
    xmlIndentTreeOutput = 1;

    /* Save document | 保存文档 */
    if (xmlSaveFormatFileEnc(opts->output_file, base_doc, "UTF-8", format_output) < 0) {
        printf("Error: Cannot save output file '%s'\n", opts->output_file);
        xmlFreeDoc(base_doc);
        return 0;
    }
    
    xmlFreeDoc(base_doc);
    /* Print completion message | 打印完成消息 */
    if (opts->input_file_count > 1) {
        printf("Merge completed, output file: %s\n", opts->output_file);
    }
    return 1;
} 