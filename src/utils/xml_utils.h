#ifndef XML_UTILS_H
#define XML_UTILS_H

#include <libxml/parser.h>
#include "../main/common.h"

/* Detected indentation information | 检测到的缩进信息 */
typedef struct {
    char style;         /* 't' for tab, 's' for space | 't'表示tab，'s'表示空格 */
    int width;          /* For spaces, number of spaces; for tab, always 1 | 对于空格是空格数，对于tab永远是1 */
} DetectedIndentStyle;

/* Compare node names ignoring namespaces | 比较节点名称（忽略命名空间） */
int compare_node_names(const xmlChar* name1, const xmlChar* name2);

/* Get SHORT-NAME content from node | 从节点获取SHORT-NAME内容 */
xmlChar* get_short_name(xmlNodePtr node);

/* Count SHORT-NAME nodes in node and all its children | 统计节点及其所有子节点中的SHORT-NAME节点数量 */
int count_short_names(xmlNodePtr node);

/* Sort nodes by SHORT-NAME | 按SHORT-NAME对节点进行排序 */
void sort_nodes_by_short_name(xmlNodePtr parent, SortOrder order);

/* Detect indentation style from XML file | 从XML文件中检测缩进风格 */
DetectedIndentStyle detect_indent_style(const char* filename);

/* Sort children of specific tag by SHORT-NAME | 对特定标签的子节点按SHORT-NAME排序 */
int sort_specific_tag_children(xmlNodePtr root, const char* tag_name, SortOrder order);

#endif /* XML_UTILS_H */ 