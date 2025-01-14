#include "xml_utils.h"
#include <libxml/parser.h>
#include <string.h>

/* Compare node names ignoring namespaces | 比较节点名称（忽略命名空间） */
int compare_node_names(const xmlChar* name1, const xmlChar* name2) {
    /* Skip namespace prefix | 跳过命名空间前缀 */
    const xmlChar* local_name1 = xmlStrchr(name1, ':');
    const xmlChar* local_name2 = xmlStrchr(name2, ':');
    
    /* If no namespace prefix, use full name | 如果没有命名空间前缀，使用完整名称 */
    if (local_name1 == NULL) local_name1 = name1;
    else local_name1++; /* Skip colon | 跳过冒号 */
    if (local_name2 == NULL) local_name2 = name2;
    else local_name2++; /* Skip colon | 跳过冒号 */
    
    return xmlStrcmp(local_name1, local_name2);
}

/* Get SHORT-NAME content from node | 从节点获取SHORT-NAME内容 */
xmlChar* get_short_name(xmlNodePtr node) {
    xmlNodePtr cur = node->children;
    while (cur != NULL) {
        /* Find SHORT-NAME node and return its content | 查找SHORT-NAME节点并返回其内容 */
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"SHORT-NAME"))) {
            return xmlNodeGetContent(cur);
        }
        cur = cur->next;
    }
    return NULL;
}

/* Count SHORT-NAME nodes in node and all its children | 统计节点及其所有子节点中的SHORT-NAME节点数量 */
int count_short_names(xmlNodePtr node) {
    /* If not an element node, return 0 | 如果不是元素节点，返回0 */
    if (node->type != XML_ELEMENT_NODE) {
        return 0;
    }
    
    int count = 0;
    xmlNodePtr cur = node->children;
    
    /* Recursively check all child nodes | 递归检查所有子节点 */
    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"SHORT-NAME"))) {
            count++;
            if (count > 1) {
                return count;  /* If multiple found, return immediately | 如果找到多个，立即返回 */
            }
        } else if (cur->type == XML_ELEMENT_NODE) {
            count += count_short_names(cur);
            if (count > 1) {
                return count;  /* If multiple found, return immediately | 如果找到多个，立即返回 */
            }
        }
        cur = cur->next;
    }
    
    return count;
} 