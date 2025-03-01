#include "xml_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/parser.h>

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

/* Compare nodes by SHORT-NAME | 按SHORT-NAME比较节点 */
static int compare_nodes(xmlNodePtr node1, xmlNodePtr node2, SortOrder order) {
    xmlChar *name1 = get_short_name(node1);
    xmlChar *name2 = get_short_name(node2);
    int result;

    /* If either node has no SHORT-NAME, don't change order | 如果任一节点没有SHORT-NAME，保持原顺序 */
    if (!name1 || !name2) {
        if (name1) xmlFree(name1);
        if (name2) xmlFree(name2);
        return 0;
    }

    /* Compare SHORT-NAMEs | 比较SHORT-NAME */
    result = xmlStrcmp(name1, name2);
    
    /* Free memory | 释放内存 */
    xmlFree(name1);
    xmlFree(name2);

    /* Apply sort order | 应用排序方式 */
    return order == SORT_ASC ? result : -result;
}

/* Sort sibling nodes | 对兄弟节点排序 */
static void sort_sibling_nodes(xmlNodePtr first, SortOrder order) {
    if (!first) return;

    /* Convert linked list to array for easier sorting | 将链表转换为数组以便排序 */
    xmlNodePtr *nodes = NULL;
    int count = 0;
    xmlNodePtr current = first;

    /* Count nodes | 统计节点数量 */
    while (current) {
        if (current->type == XML_ELEMENT_NODE) {
            count++;
        }
        current = current->next;
    }

    if (count == 0) return;

    /* Allocate array | 分配数组 */
    nodes = (xmlNodePtr*)malloc(count * sizeof(xmlNodePtr));
    if (!nodes) return;

    /* Fill array | 填充数组 */
    current = first;
    int i = 0;
    while (current && i < count) {
        if (current->type == XML_ELEMENT_NODE) {
            nodes[i++] = current;
        }
        current = current->next;
    }

    /* Sort array | 排序数组 */
    for (i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (compare_nodes(nodes[j], nodes[j + 1], order) > 0) {
                xmlNodePtr temp = nodes[j];
                nodes[j] = nodes[j + 1];
                nodes[j + 1] = temp;
            }
        }
    }

    /* Relink nodes in sorted order | 按排序顺序重新链接节点 */
    xmlNodePtr parent = first->parent;
    for (i = 0; i < count; i++) {
        xmlUnlinkNode(nodes[i]);
        xmlAddChild(parent, nodes[i]);
    }

    /* Free array | 释放数组 */
    free(nodes);
}

/* Sort nodes by SHORT-NAME | 按SHORT-NAME对节点进行排序 */
void sort_nodes_by_short_name(xmlNodePtr parent, SortOrder order) {
    if (!parent) return;

    /* First, recursively process child nodes | 首先递归处理子节点 */
    xmlNodePtr node = parent->children;
    while (node) {
        xmlNodePtr next = node->next; /* Store next before sorting | 在排序前保存next指针 */
        if (node->type == XML_ELEMENT_NODE) {
            /* Only process nodes that have multiple SHORT-NAMEs | 只处理有多个SHORT-NAME的节点 */
            if (count_short_names(node) > 1) {
                sort_nodes_by_short_name(node, order);
            }
        }
        node = next;
    }

    /* Then sort children of current node | 然后对当前节点的子节点排序 */
    if (parent->children) {
        sort_sibling_nodes(parent->children, order);
    }
}

/* Check if line contains XML element | 检查行是否包含XML元素 */
static int is_element_line(const char* line) {
    /* Skip leading whitespace | 跳过前导空白 */
    while (*line == ' ' || *line == '\t') line++;
    
    /* Check if it's an element | 检查是否是元素 */
    return (*line == '<' && line[1] != '?' && line[1] != '!');
}

/* Get indentation of line | 获取行的缩进信息 */
static int get_line_indent(const char* line, char* style) {
    int count = 0;
    *style = 's';  /* Default to space | 默认为空格 */
    
    while (*line == ' ' || *line == '\t') {
        if (*line == '\t') {
            *style = 't';
            return 1;  /* Tab found | 发现tab */
        }
        count++;
        line++;
    }
    
    return count;
}

DetectedIndentStyle detect_indent_style(const char* filename) {
    DetectedIndentStyle style = {.style = 's', .width = 4};  /* Default: 4 spaces | 默认4空格 */
    FILE* file = fopen(filename, "r");
    if (!file) {
        return style;
    }

    char prev_line[1024] = "";
    char curr_line[1024];
    int found_indent = 0;
    int line_num = 0;
    
    /* Get first line | 获取第一行 */
    if (!fgets(prev_line, sizeof(prev_line), file)) {
        fclose(file);
        return style;
    }
    line_num++;
    
    /* Compare consecutive lines | 比较连续的行 */
    while (fgets(curr_line, sizeof(curr_line), file)) {
        line_num++;
        
        /* If both lines are elements | 如果两行都是元素 */
        if (is_element_line(prev_line) && is_element_line(curr_line)) {
            char prev_style, curr_style;
            int prev_indent = get_line_indent(prev_line, &prev_style);
            int curr_indent = get_line_indent(curr_line, &curr_style);
            
            /* If current line is more indented | 如果当前行缩进更多 */
            if (curr_indent > prev_indent) {
                if (curr_style == 't') {
                    style.style = 't';
                    style.width = 1;
                    found_indent = 1;
                    break;
                } else if (!found_indent) {
                    int width = curr_indent - prev_indent;
                    if (width > 0 && width <= 8) {
                        style.style = 's';
                        style.width = width;
                        found_indent = 1;
                        break;
                    }
                }
            }
        }
        
        /* Move current line to previous | 当前行移动到前一行 */
        strcpy(prev_line, curr_line);
    }
    
    fclose(file);
    return style;
} 

/* Sort children of specific tag by SHORT-NAME | 对特定标签的子节点按SHORT-NAME排序 */
int sort_specific_tag_children(xmlNodePtr root, const char* tag_name, SortOrder order) {
    if (!root || !tag_name) return 0;
    int count = 0;

    /* 递归处理所有节点 */
    xmlNodePtr node = root;
    while (node) {
        if (node->type == XML_ELEMENT_NODE) {
            /* 如果找到匹配的标签，处理其子节点并增加计数 */
            if (xmlStrcmp(node->name, (const xmlChar*)tag_name) == 0) {
                if (node->children) {
                    sort_sibling_nodes(node->children, order);
                }
                count++;
            }
            /* 递归处理子节点 */
            count += sort_specific_tag_children(node->children, tag_name, order);
        }
        node = node->next;
    }

    return count;
}