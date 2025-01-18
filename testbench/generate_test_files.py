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
            create_random_node(child, depth + 1, max_depth)

def generate_arxml(filename, max_depth, min_children=2, max_children=5):
    """Generate an ARXML file | 生成ARXML文件"""
    print(f"Generating file: {filename}")
    root = Element("AUTOSAR")
    
    # Add xmlns and schema version | 添加xmlns和架构版本
    root.set("xmlns", "http://autosar.org/schema/r4.0")
    root.set("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")
    root.set("xsi:schemaLocation", "http://autosar.org/schema/r4.0 AUTOSAR_4-0-3.xsd")
    
    # Create random structure | 创建随机结构
    create_random_node(root, 0, max_depth, min_children, max_children)
    
    # Create pretty XML | 创建美观的XML
    xml_str = xml.dom.minidom.parseString(
        '<?xml version="1.0" encoding="UTF-8"?>' + 
        tostring(root, encoding='unicode')
    ).toprettyxml()
    
    # Create directory if it doesn't exist | 如果目录不存在则创建
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    
    # Save to file | 保存到文件
    with open(filename, 'w', encoding='utf-8') as f:
        f.write(xml_str)
    print(f"Generated file: {filename}")

def main():
    """Main function to generate test files | 生成测试文件的主函数"""
    # Get script directory | 获取脚本目录
    script_dir = os.path.dirname(os.path.abspath(__file__))
    print(f"Script directory: {script_dir}")
    
    # Create test directories | 创建测试目录
    test_case_dir = os.path.join(script_dir, "cases", "6.1")
    test_result_dir = os.path.join(script_dir, "results", "6.1")
    
    print(f"Creating directories:")
    print(f"  Test cases: {test_case_dir}")
    print(f"  Test results: {test_result_dir}")
    
    os.makedirs(test_case_dir, exist_ok=True)
    os.makedirs(test_result_dir, exist_ok=True)
    
    # Generate small files (depth 3-4) | 生成小文件
    print("\nGenerating small files...")
    for i in range(1, 3):
        generate_arxml(os.path.join(test_case_dir, f"small{i}.arxml"), 
                      max_depth=3, min_children=2, max_children=3)
    
    # Generate medium files (depth 5-6) | 生成中等文件
    print("\nGenerating medium files...")
    for i in range(1, 6):
        generate_arxml(os.path.join(test_case_dir, f"med{i}.arxml"), 
                      max_depth=5, min_children=3, max_children=5)
    
    # Generate large files (depth 7-8) | 生成大文件
    print("\nGenerating large files...")
    for i in range(1, 11):
        generate_arxml(os.path.join(test_case_dir, f"large{i}.arxml"), 
                      max_depth=7, min_children=4, max_children=7)
    
    # Generate deep nesting files (depth 10) | 生成深层嵌套文件
    print("\nGenerating deep nesting files...")
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