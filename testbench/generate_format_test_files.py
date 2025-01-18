#!/usr/bin/env python3

import os
import random
from xml.etree.ElementTree import Element, SubElement, ElementTree, tostring
import xml.dom.minidom

def create_random_node(parent, depth, max_depth, min_children=2, max_children=5):
    """Create a random node with children | 创建带有子节点的随机节点"""
    if depth >= max_depth:
        return
    
    # Random number of children | 随机子节点数量
    num_children = random.randint(min_children, max_children)
    
    for i in range(num_children):
        child = SubElement(parent, f"CONTAINER-{depth}-{i}")
        # Add SHORT-NAME | 添加SHORT-NAME
        short_name = SubElement(child, "SHORT-NAME")
        short_name.text = f"TestNode_{depth}_{i}"
        
        # Add some parameters | 添加一些参数
        params = SubElement(child, "PARAMETERS")
        for j in range(random.randint(1, 3)):
            param = SubElement(params, f"PARAM-{j}")
            param.text = f"Value_{j}"
        
        # Recursively create children | 递归创建子节点
        if depth < max_depth:
            create_random_node(child, depth + 1, max_depth, min_children, max_children)

def generate_arxml(filename, max_depth, indent_style='random', min_children=2, max_children=5):
    """Generate an ARXML file | 生成ARXML文件"""
    print(f"Generating file: {filename}")
    root = Element("AUTOSAR")
    
    # Add xmlns and schema version | 添加xmlns和架构版本
    root.set("xmlns", "http://autosar.org/schema/r4.0")
    root.set("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")
    root.set("xsi:schemaLocation", "http://autosar.org/schema/r4.0 AUTOSAR_4-0-3.xsd")
    
    # Create random structure | 创建随机结构
    create_random_node(root, 0, max_depth, min_children, max_children)
    
    # Create XML string with specific indentation | 创建带有特定缩进的XML字符串
    rough_xml = tostring(root, encoding='unicode')
    
    # Apply different indentation styles | 应用不同的缩进风格
    if indent_style == 'none':
        # No indentation | 无缩进
        xml_str = '<?xml version="1.0" encoding="UTF-8"?>\n' + rough_xml.replace('\n', '')
    elif indent_style == 'mixed':
        # Mixed indentation (tabs and spaces) | 混合缩进（制表符和空格）
        doc = xml.dom.minidom.parseString(rough_xml)
        lines = doc.toprettyxml().split('\n')
        xml_str = '<?xml version="1.0" encoding="UTF-8"?>\n'
        for i, line in enumerate(lines[1:], 1):  # Skip the XML declaration
            if i % 2 == 0:
                # Use tabs for even lines | 偶数行使用制表符
                xml_str += line.replace('    ', '\t') + '\n'
            else:
                # Use spaces for odd lines | 奇数行使用空格
                xml_str += line + '\n'
    else:
        # Normal indentation | 正常缩进
        doc = xml.dom.minidom.parseString(rough_xml)
        xml_str = '<?xml version="1.0" encoding="UTF-8"?>\n' + '\n'.join(doc.toprettyxml().split('\n')[1:])
    
    # Create directory if it doesn't exist | 如果目录不存在则创建
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    
    # Save to file | 保存到文件
    with open(filename, 'w', encoding='utf-8', newline='\n') as f:
        f.write(xml_str)
    print(f"Generated file: {filename}")

def main():
    """Main function to generate test files | 生成测试文件的主函数"""
    # Create test directories | 创建测试目录
    test_case_dir = os.path.join("cases", "6.2")
    test_result_dir = os.path.join("results", "6.2")
    
    print(f"Creating directories:")
    print(f"  Test cases: {test_case_dir}")
    print(f"  Test results: {test_result_dir}")
    
    os.makedirs(test_case_dir, exist_ok=True)
    os.makedirs(test_result_dir, exist_ok=True)
    
    # Generate files with different indentation styles | 生成不同缩进风格的文件
    print("\nGenerating files with no indentation...")
    for i in range(1, 3):
        generate_arxml(os.path.join(test_case_dir, f"no_indent{i}.arxml"), 
                      max_depth=3, indent_style='none')
    
    print("\nGenerating files with mixed indentation...")
    for i in range(1, 3):
        generate_arxml(os.path.join(test_case_dir, f"mixed_indent{i}.arxml"), 
                      max_depth=5, indent_style='mixed')
    
    print("\nGenerating large files...")
    for i in range(1, 4):
        generate_arxml(os.path.join(test_case_dir, f"large{i}.arxml"), 
                      max_depth=7, min_children=4, max_children=7)
    
    print("\nGenerating deep nested files...")
    for i in range(1, 3):
        generate_arxml(os.path.join(test_case_dir, f"deep{i}.arxml"), 
                      max_depth=10, min_children=2, max_children=3)
    
    print("\nTest file generation completed!")
    print(f"Files are located in: {test_case_dir}")

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"Error occurred: {str(e)}")
        import traceback
        traceback.print_exc() 