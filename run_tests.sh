#!/bin/bash

# Set script encoding to UTF-8
export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8

# Color definitions (may not be supported in Windows)
if [ "$OS" = "Windows_NT" ]; then
    GREEN=""
    RED=""
    NC=""
else
    GREEN='\033[0;32m'
    RED='\033[0;31m'
    NC='\033[0m'
fi

# Test counters
TOTAL_TESTS=0
PASSED_TESTS=0

# Create test files function
create_test_files() {
    cat > test_files/duplicate1.arxml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<AUTOSAR xmlns="http://autosar.org/schema/r4.0">
    <AR-PACKAGES>
        <AR-PACKAGE>
            <SHORT-NAME>Container1</SHORT-NAME>
            <ELEMENTS>
                <ECUC-CONTAINER-VALUE>
                    <SHORT-NAME>Config1</SHORT-NAME>
                    <PARAMETER-VALUES>
                        <ECUC-NUMERICAL-PARAM-VALUE>
                            <VALUE>42</VALUE>
                        </ECUC-NUMERICAL-PARAM-VALUE>
                    </PARAMETER-VALUES>
                </ECUC-CONTAINER-VALUE>
            </ELEMENTS>
        </AR-PACKAGE>
    </AR-PACKAGES>
</AUTOSAR>
EOF

    cat > test_files/duplicate2.arxml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<AUTOSAR xmlns="http://autosar.org/schema/r4.0">
    <AR-PACKAGES>
        <AR-PACKAGE>
            <SHORT-NAME>Container1</SHORT-NAME>
            <ELEMENTS>
                <ECUC-CONTAINER-VALUE>
                    <SHORT-NAME>Config1</SHORT-NAME>
                    <PARAMETER-VALUES>
                        <ECUC-NUMERICAL-PARAM-VALUE>
                            <VALUE>100</VALUE>
                        </ECUC-NUMERICAL-PARAM-VALUE>
                    </PARAMETER-VALUES>
                </ECUC-CONTAINER-VALUE>
            </ELEMENTS>
        </AR-PACKAGE>
    </AR-PACKAGES>
</AUTOSAR>
EOF

    cat > test_files/duplicate3.arxml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<AUTOSAR xmlns="http://autosar.org/schema/r4.0">
    <AR-PACKAGES>
        <AR-PACKAGE>
            <SHORT-NAME>Container2</SHORT-NAME>
            <ELEMENTS>
                <ECUC-CONTAINER-VALUE>
                    <SHORT-NAME>Config2</SHORT-NAME>
                    <PARAMETER-VALUES>
                        <ECUC-NUMERICAL-PARAM-VALUE>
                            <VALUE>200</VALUE>
                        </ECUC-NUMERICAL-PARAM-VALUE>
                    </PARAMETER-VALUES>
                </ECUC-CONTAINER-VALUE>
            </ELEMENTS>
        </AR-PACKAGE>
    </AR-PACKAGES>
</AUTOSAR>
EOF

    # Test Case 3: Multiple duplicates (3 or more occurrences)
    cat > test_files/multiple1.arxml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<AUTOSAR xmlns="http://autosar.org/schema/r4.0">
    <AR-PACKAGES>
        <AR-PACKAGE>
            <SHORT-NAME>Container1</SHORT-NAME>
            <ELEMENTS>
                <ECUC-CONTAINER-VALUE>
                    <SHORT-NAME>Config1</SHORT-NAME>
                    <VALUE>42</VALUE>
                </ECUC-CONTAINER-VALUE>
            </ELEMENTS>
        </AR-PACKAGE>
    </AR-PACKAGES>
</AUTOSAR>
EOF

    cat > test_files/multiple2.arxml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<AUTOSAR xmlns="http://autosar.org/schema/r4.0">
    <AR-PACKAGES>
        <AR-PACKAGE>
            <SHORT-NAME>Container1</SHORT-NAME>
            <ELEMENTS>
                <ECUC-CONTAINER-VALUE>
                    <SHORT-NAME>Config1</SHORT-NAME>
                    <VALUE>100</VALUE>
                </ECUC-CONTAINER-VALUE>
            </ELEMENTS>
        </AR-PACKAGE>
    </AR-PACKAGES>
</AUTOSAR>
EOF

    cat > test_files/multiple3.arxml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<AUTOSAR xmlns="http://autosar.org/schema/r4.0">
    <AR-PACKAGES>
        <AR-PACKAGE>
            <SHORT-NAME>Container1</SHORT-NAME>
            <ELEMENTS>
                <ECUC-CONTAINER-VALUE>
                    <SHORT-NAME>Config1</SHORT-NAME>
                    <VALUE>200</VALUE>
                </ECUC-CONTAINER-VALUE>
            </ELEMENTS>
        </AR-PACKAGE>
    </AR-PACKAGES>
</AUTOSAR>
EOF

    # Test Case 4: Same node name but different content structure
    cat > test_files/structure1.arxml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<AUTOSAR xmlns="http://autosar.org/schema/r4.0">
    <AR-PACKAGES>
        <AR-PACKAGE>
            <SHORT-NAME>Container1</SHORT-NAME>
            <ELEMENTS>
                <ECUC-CONTAINER-VALUE>
                    <SHORT-NAME>Config1</SHORT-NAME>
                    <PARAMETERS>
                        <ECUC-INTEGER-PARAM-VALUE>
                            <VALUE>42</VALUE>
                        </ECUC-INTEGER-PARAM-VALUE>
                    </PARAMETERS>
                </ECUC-CONTAINER-VALUE>
            </ELEMENTS>
        </AR-PACKAGE>
    </AR-PACKAGES>
</AUTOSAR>
EOF

    cat > test_files/structure2.arxml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<AUTOSAR xmlns="http://autosar.org/schema/r4.0">
    <AR-PACKAGES>
        <AR-PACKAGE>
            <SHORT-NAME>Container1</SHORT-NAME>
            <ELEMENTS>
                <ECUC-CONTAINER-VALUE>
                    <SHORT-NAME>Config1</SHORT-NAME>
                    <PARAMETERS>
                        <ECUC-STRING-PARAM-VALUE>
                            <VALUE>test</VALUE>
                        </ECUC-STRING-PARAM-VALUE>
                    </PARAMETERS>
                </ECUC-CONTAINER-VALUE>
            </ELEMENTS>
        </AR-PACKAGE>
    </AR-PACKAGES>
</AUTOSAR>
EOF

    # Test Case 5: Command file input
    cat > test_files/command.txt << 'EOF'
merge -a test_files/duplicate1.arxml -a test_files/duplicate2.arxml -m test_files/merged_from_file.arxml
EOF

    # Test Case 6: Merge non-conflicting containers
    cat > test_files/noconflict1.arxml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<AUTOSAR xmlns="http://autosar.org/schema/r4.0">
    <AR-PACKAGES>
        <AR-PACKAGE>
            <SHORT-NAME>Container1</SHORT-NAME>
            <ELEMENTS>
                <ECUC-CONTAINER-VALUE>
                    <SHORT-NAME>Config1</SHORT-NAME>
                    <PARAMETERS>
                        <ECUC-INTEGER-PARAM-VALUE>
                            <VALUE>42</VALUE>
                        </ECUC-INTEGER-PARAM-VALUE>
                    </PARAMETERS>
                </ECUC-CONTAINER-VALUE>
            </ELEMENTS>
        </AR-PACKAGE>
    </AR-PACKAGES>
</AUTOSAR>
EOF

    cat > test_files/noconflict2.arxml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<AUTOSAR xmlns="http://autosar.org/schema/r4.0">
    <AR-PACKAGES>
        <AR-PACKAGE>
            <SHORT-NAME>Container2</SHORT-NAME>
            <ELEMENTS>
                <ECUC-CONTAINER-VALUE>
                    <SHORT-NAME>Config2</SHORT-NAME>
                    <PARAMETERS>
                        <ECUC-INTEGER-PARAM-VALUE>
                            <VALUE>100</VALUE>
                        </ECUC-INTEGER-PARAM-VALUE>
                    </PARAMETERS>
                </ECUC-CONTAINER-VALUE>
            </ELEMENTS>
        </AR-PACKAGE>
    </AR-PACKAGES>
</AUTOSAR>
EOF

    cat > test_files/noconflict3.arxml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<AUTOSAR xmlns="http://autosar.org/schema/r4.0">
    <AR-PACKAGES>
        <AR-PACKAGE>
            <SHORT-NAME>Container3</SHORT-NAME>
            <ELEMENTS>
                <ECUC-CONTAINER-VALUE>
                    <SHORT-NAME>Config3</SHORT-NAME>
                    <PARAMETERS>
                        <ECUC-INTEGER-PARAM-VALUE>
                            <VALUE>200</VALUE>
                        </ECUC-INTEGER-PARAM-VALUE>
                    </PARAMETERS>
                </ECUC-CONTAINER-VALUE>
            </ELEMENTS>
        </AR-PACKAGE>
    </AR-PACKAGES>
</AUTOSAR>
EOF

    # 添加缩进测试用的文件
    cat > test_files/indent_test.arxml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<AUTOSAR xmlns="http://autosar.org/schema/r4.0">
    <AR-PACKAGES>
        <AR-PACKAGE>
            <SHORT-NAME>IndentTest</SHORT-NAME>
            <ELEMENTS>
                <ECUC-CONTAINER-VALUE>
                    <SHORT-NAME>TestConfig</SHORT-NAME>
                    <PARAMETERS>
                        <ECUC-INTEGER-PARAM-VALUE>
                            <VALUE>42</VALUE>
                        </ECUC-INTEGER-PARAM-VALUE>
                    </PARAMETERS>
                </ECUC-CONTAINER-VALUE>
            </ELEMENTS>
        </AR-PACKAGE>
    </AR-PACKAGES>
</AUTOSAR>
EOF

    # 添加 format 测试用的文件
    cat > test_files/format_test.arxml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<AUTOSAR xmlns="http://autosar.org/schema/r4.0">
<AR-PACKAGES>
<AR-PACKAGE>
<SHORT-NAME>FormatTest</SHORT-NAME>
<ELEMENTS>
<ECUC-CONTAINER-VALUE>
<SHORT-NAME>NoIndent</SHORT-NAME>
<PARAMETERS>
<ECUC-INTEGER-PARAM-VALUE>
<VALUE>42</VALUE>
</ECUC-INTEGER-PARAM-VALUE>
</PARAMETERS>
</ECUC-CONTAINER-VALUE>
</ELEMENTS>
</AR-PACKAGE>
</AR-PACKAGES>
</AUTOSAR>
EOF

    # 创建用于测试输出目录的文件夹
    mkdir -p test_files/format_output
}

# Main test process
# 1. Clean and compile
rm -rf test_files
rm -f arXmlTool.exe
echo "Compiling program..."
if [ "$OS" = "Windows_NT" ]; then
    # Windows environment (assuming MinGW)
    XML2_PATH="C:/msys64/mingw64"
    gcc -Wall -Wextra \
        -I"$XML2_PATH/include/libxml2" \
        -o arXmlTool.exe arXmlTool.c \
        -static \
        -L"$XML2_PATH/lib" \
        -lxml2 -liconv -lz -llzma -lws2_32
else
    # Linux/Unix environment
    CFLAGS=$(pkg-config --cflags libxml-2.0)
    LIBS=$(pkg-config --libs libxml-2.0)
    gcc -Wall -Wextra $CFLAGS -o arXmlTool.exe arXmlTool.c $LIBS
fi

if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

# Check if executable exists and add execution permission
if [ ! -f "arXmlTool.exe" ]; then
    echo "Error: Executable file not found after compilation!"
    exit 1
fi
chmod +x arXmlTool.exe

# 2. Create test environment
echo "Preparing test environment..."
mkdir -p test_files
create_test_files

# 3. Execute tests
echo "Starting tests..."
echo "-------------------"

# Test cases
echo "Test Case 1: Display Help Information"
echo "Command: ./arXmlTool.exe"
./arXmlTool.exe
echo "Verification Criteria:"
echo "1. Should display usage information"
echo "2. Should show all available modes (merge, compare, generate)"
echo "3. Should show all merge mode options (-a, -m, -o)"
echo ""

echo "Test Case 2: Basic Merge with Duplicates"
echo "Command: ./arXmlTool.exe merge -a test_files/duplicate1.arxml -a test_files/duplicate2.arxml -m test_files/merged_duplicate.arxml"
./arXmlTool.exe merge -a test_files/duplicate1.arxml -a test_files/duplicate2.arxml -m test_files/merged_duplicate.arxml
echo "Verification Criteria:"
echo "1. Check merged_duplicate.arxml content:"
echo "   - Should have only one Container1"
echo "   - Should have only one Config1"
echo "   - Should have only one value 42"
echo "   - Should not contain value 100"
echo "2. Check merged file content:"
cat test_files/merged_duplicate.arxml
echo ""

echo "Test Case 3: Merge Multiple Duplicates"
echo "Command: ./arXmlTool.exe merge -a test_files/multiple1.arxml -a test_files/multiple2.arxml -a test_files/multiple3.arxml -m test_files/merged_multiple.arxml"
./arXmlTool.exe merge -a test_files/multiple1.arxml -a test_files/multiple2.arxml -a test_files/multiple3.arxml -m test_files/merged_multiple.arxml
echo "Verification Criteria:"
echo "1. Check merged_multiple.arxml content:"
echo "   - Should have only one Container1"
echo "   - Should have only one Config1"
echo "   - Should have value 42 (first occurrence)"
echo "   - Should not contain values 100 or 200"
echo "2. Check merged file content:"
cat test_files/merged_multiple.arxml
echo ""

echo "Test Case 4: Merge Different Structures"
echo "Command: ./arXmlTool.exe merge -a test_files/structure1.arxml -a test_files/structure2.arxml -m test_files/merged_structure.arxml"
./arXmlTool.exe merge -a test_files/structure1.arxml -a test_files/structure2.arxml -m test_files/merged_structure.arxml
echo "Verification Criteria:"
echo "1. Check merged_structure.arxml content:"
echo "   - Should have only one Container1"
echo "   - Should have only one Config1"
echo "   - Should keep ECUC-INTEGER-PARAM-VALUE from first file"
echo "   - Should not contain ECUC-STRING-PARAM-VALUE"
echo "2. Check merged file content:"
cat test_files/merged_structure.arxml
echo ""

echo "Test Case 5: Command File Input"
echo "Command: ./arXmlTool.exe -f test_files/command.txt"
./arXmlTool.exe -f test_files/command.txt
echo "Verification Criteria:"
echo "1. Check merged_from_file.arxml content:"
echo "   - Should be identical to merged_duplicate.arxml"
echo "2. Check merged file content:"
cat test_files/merged_from_file.arxml
echo ""

echo "Test Case 6: Merge Non-conflicting Containers"
echo "Command: ./arXmlTool.exe merge -a test_files/noconflict1.arxml -a test_files/noconflict2.arxml -a test_files/noconflict3.arxml -m test_files/merged_noconflict.arxml"
./arXmlTool.exe merge -a test_files/noconflict1.arxml -a test_files/noconflict2.arxml -a test_files/noconflict3.arxml -m test_files/merged_noconflict.arxml
echo "Verification Criteria:"
echo "1. Check merged_noconflict.arxml content:"
echo "   - Should have Container1 with Config1 and value 42"
echo "   - Should have Container2 with Config2 and value 100"
echo "   - Should have Container3 with Config3 and value 200"
echo "   - All containers should maintain their original structure"
echo "   - Order of containers should match input order"
echo "2. Check merged file content:"
cat test_files/merged_noconflict.arxml
echo ""

echo "Test Case 7: Indentation Tests"
echo "-------------------"

echo "Test Case 7.1: Tab Indentation"
echo "Command: ./arXmlTool.exe merge -a test_files/indent_test.arxml -m test_files/merged_tab.arxml -i tab"
./arXmlTool.exe merge -a test_files/indent_test.arxml -m test_files/merged_tab.arxml -i tab
echo "Verification Criteria:"
echo "1. Check merged_tab.arxml content:"
echo "   - Should use tab characters for indentation"
echo "   - Structure should be preserved"
echo "2. Check indentation (showing first few lines):"
echo "File content with visible tabs:"
sed 's/\t/[TAB]/g' test_files/merged_tab.arxml | head -n 10
echo ""

echo "Test Case 7.2: 2-Space Indentation"
echo "Command: ./arXmlTool.exe merge -a test_files/indent_test.arxml -m test_files/merged_2space.arxml -i 2"
./arXmlTool.exe merge -a test_files/indent_test.arxml -m test_files/merged_2space.arxml -i 2
echo "Verification Criteria:"
echo "1. Check merged_2space.arxml content:"
echo "   - Should use 2 spaces for indentation"
echo "   - Structure should be preserved"
echo "2. Check indentation (showing first few lines):"
head -n 10 test_files/merged_2space.arxml
echo ""

echo "Test Case 7.3: 4-Space Indentation (Default)"
echo "Command: ./arXmlTool.exe merge -a test_files/indent_test.arxml -m test_files/merged_4space.arxml -i 4"
./arXmlTool.exe merge -a test_files/indent_test.arxml -m test_files/merged_4space.arxml -i 4
echo "Verification Criteria:"
echo "1. Check merged_4space.arxml content:"
echo "   - Should use 4 spaces for indentation"
echo "   - Structure should be preserved"
echo "2. Check indentation (showing first few lines):"
head -n 10 test_files/merged_4space.arxml
echo ""

echo "Test Case 7.4: Invalid Indentation Style"
echo "Command: ./arXmlTool.exe merge -a test_files/indent_test.arxml -m test_files/merged_invalid.arxml -i 3"
./arXmlTool.exe merge -a test_files/indent_test.arxml -m test_files/merged_invalid.arxml -i 3
echo "Verification Criteria:"
echo "1. Should show error message about invalid indentation style"
echo "2. Should not create output file"
echo ""

echo "Test Case 8: Format Mode Tests"
echo "-------------------"

echo "Test Case 8.1: Format with Tab Indentation (In-place)"
echo "Command: ./arXmlTool.exe format -a test_files/format_test.arxml -i tab"
cp test_files/format_test.arxml test_files/format_test_tab.arxml
./arXmlTool.exe format -a test_files/format_test_tab.arxml -i tab
echo "Verification Criteria:"
echo "1. Check formatted file content:"
echo "   - Should use tab characters for indentation"
echo "   - Original file structure should be preserved"
echo "2. Check indentation (showing first few lines with visible tabs):"
sed 's/\t/[TAB]/g' test_files/format_test_tab.arxml | head -n 10
echo ""

echo "Test Case 8.2: Format with 2-Space Indentation (Output Directory)"
echo "Command: ./arXmlTool.exe format -a test_files/format_test.arxml -i 2 -o test_files/format_output"
./arXmlTool.exe format -a test_files/format_test.arxml -i 2 -o test_files/format_output
echo "Verification Criteria:"
echo "1. Check formatted file content:"
echo "   - Should use 2 spaces for indentation"
echo "   - Original file should remain unchanged"
echo "   - New file should be created in format_output directory"
echo "2. Check indentation (showing first few lines):"
head -n 10 test_files/format_output/format_test.arxml
echo "3. Compare with original file:"
echo "Original file first line indentation:"
head -n 3 test_files/format_test.arxml
echo "Formatted file first line indentation:"
head -n 3 test_files/format_output/format_test.arxml
echo ""

echo "Test Case 8.3: Format Multiple Files"
echo "Command: ./arXmlTool.exe format -a test_files/format_test.arxml -a test_files/indent_test.arxml -i 4 -o test_files/format_output/case8.3"
# 创建专门的测试目录
mkdir -p test_files/format_output/case8.3
./arXmlTool.exe format -a test_files/format_test.arxml -a test_files/indent_test.arxml -i 4 -o test_files/format_output/case8.3
echo "Verification Criteria:"
echo "1. Check if both files are formatted:"
echo "   - Both files should use 4 spaces for indentation"
echo "   - Both files should be in format_output/case8.3 directory"
echo "2. Check files existence and content:"
ls -l test_files/format_output/case8.3/format_test.arxml test_files/format_output/case8.3/indent_test.arxml
echo "3. Check indentation of first file:"
head -n 10 test_files/format_output/case8.3/format_test.arxml
echo "4. Check indentation of second file:"
head -n 10 test_files/format_output/case8.3/indent_test.arxml
echo ""

echo "Test Case 8.4: Format with Invalid Style"
echo "Command: ./arXmlTool.exe format -a test_files/format_test.arxml -i 3"
./arXmlTool.exe format -a test_files/format_test.arxml -i 3
echo "Verification Criteria:"
echo "1. Should show error message about invalid indentation style"
echo "2. Original file should remain unchanged"
echo ""

echo "Test Case 8.5: Format Using Command File"
# Create command file for format mode
cat > test_files/format_command.txt << 'EOF'
format -a test_files/format_test.arxml -i tab -o test_files/format_output/case8.5
EOF
# 创建专门的测试目录
mkdir -p test_files/format_output/case8.5
echo "Command: ./arXmlTool.exe -f test_files/format_command.txt"
./arXmlTool.exe -f test_files/format_command.txt
echo "Verification Criteria:"
echo "1. Check if command file is processed correctly"
echo "2. Check if file is formatted with tab indentation"
echo "3. Check formatted file location and content:"
ls -l test_files/format_output/case8.5/format_test.arxml
echo "4. Check indentation (showing first few lines with visible tabs):"
sed 's/\t/[TAB]/g' test_files/format_output/case8.5/format_test.arxml | head -n 5
echo ""

echo "Test Case 8.6: Format Without Output Directory (In-place Modification)"
cp test_files/format_test.arxml test_files/format_test_inplace.arxml
echo "Command: ./arXmlTool.exe format -a test_files/format_test_inplace.arxml -i 4"
./arXmlTool.exe format -a test_files/format_test_inplace.arxml -i 4
echo "Verification Criteria:"
echo "1. Check if original file is modified in place"
echo "2. Check indentation of modified file:"
head -n 10 test_files/format_test_inplace.arxml
echo "" 