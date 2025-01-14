#ifndef XML_UTILS_H
#define XML_UTILS_H

#include <libxml/parser.h>

/* Compare node names ignoring namespaces | 比较节点名称（忽略命名空间） */
int compare_node_names(const xmlChar* name1, const xmlChar* name2);

/* Get SHORT-NAME content from node | 从节点获取SHORT-NAME内容 */
xmlChar* get_short_name(xmlNodePtr node);

/* Count SHORT-NAME nodes in node and all its children | 统计节点及其所有子节点中的SHORT-NAME节点数量 */
int count_short_names(xmlNodePtr node);

#endif /* XML_UTILS_H */ 