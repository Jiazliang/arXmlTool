# Format Performance test script for arXmlTool (PowerShell version)
# 格式化性能测试脚本（PowerShell版本）

# Test directories | 测试目录
$TEST_CASE_DIR = "cases/6.2"
$TEST_RESULT_DIR = "results/6.2"

# Number of test iterations | 测试迭代次数
$TEST_ITERATIONS = 3

# Function to get file size in MB | 获取文件大小（MB）
function Get-FileSizeMB {
    param ([string]$FilePath)
    $size = (Get-Item $FilePath).Length
    return [math]::Round($size / 1MB, 2)
}

# Function to get size range of files | 获取文件大小范围
function Get-FileSizeRange {
    param ([string[]]$FilePaths)
    $sizes = $FilePaths | ForEach-Object { Get-FileSizeMB $_ }
    $min = ($sizes | Measure-Object -Minimum).Minimum
    $max = ($sizes | Measure-Object -Maximum).Maximum
    if ($min -eq $max) {
        return "$min MB"
    }
    return "$min - $max MB"
}

# Function to measure execution time and resource usage
# 测量执行时间和资源使用的函数
function Measure-ToolPerformance {
    param (
        [string]$TestName,
        [string]$Command,
        [string[]]$InputFiles,
        [string]$IndentStyle = "4",  # Default to 4 spaces
        [int]$Iterations = $TEST_ITERATIONS
    )
    
    Write-Host "`nRunning test: $TestName ($Iterations iterations)"
    Write-Host "Command: $Command"
    
    # Get file info | 获取文件信息
    $fileCount = $InputFiles.Count
    $sizeRange = Get-FileSizeRange $InputFiles
    Write-Host "Files: $fileCount files, Size range: $sizeRange"
    Write-Host "Indent style: $IndentStyle"
    
    $allDurations = @()
    $allMemoryPeaks = @()
    
    # Run multiple iterations | 运行多次迭代
    for ($i = 1; $i -le $Iterations; $i++) {
        Write-Host "  Iteration $i/$Iterations..."
        
        # Start process and measure | 启动进程并测量
        $startTime = Get-Date
        $process = Start-Process -FilePath "..\build\arXmlTool.exe" -ArgumentList $Command -Wait -PassThru -NoNewWindow
        $duration = ((Get-Date) - $startTime).TotalSeconds
        
        # Record results | 记录结果
        $allDurations += $duration
        $allMemoryPeaks += $process.PeakWorkingSet64
    }
    
    # Calculate averages | 计算平均值
    $avgDuration = ($allDurations | Measure-Object -Average).Average
    $avgMemory = ($allMemoryPeaks | Measure-Object -Average).Average / 1KB
    
    # Calculate standard deviation | 计算标准差
    $durationStdDev = [Math]::Sqrt(($allDurations | ForEach-Object { [Math]::Pow($_ - $avgDuration, 2) } | Measure-Object -Average).Average)
    
    # Print results | 打印结果
    Write-Host "Results for: $TestName"
    Write-Host "  Average execution time: $([Math]::Round($avgDuration, 3)) seconds (±$([Math]::Round($durationStdDev, 3)))"
    Write-Host "  Peak memory usage: $([Math]::Round($avgMemory, 2)) KB"
    Write-Host "  Exit code: $($process.ExitCode)"
    Write-Host "----------------------------------------"
    
    # Save results to CSV | 保存结果到CSV
    "$TestName,$([Math]::Round($avgDuration, 3)),$([Math]::Round($durationStdDev, 3)),$([Math]::Round($avgMemory, 2)),$($process.ExitCode),$fileCount,$sizeRange,$IndentStyle" | 
        Add-Content -Path "$TEST_RESULT_DIR\format_performance_results.csv"
    
    return @{
        TestName = $TestName
        Duration = $avgDuration
        DurationStdDev = $durationStdDev
        Memory = $avgMemory
        ExitCode = $process.ExitCode
        FileCount = $fileCount
        SizeRange = $sizeRange
        IndentStyle = $IndentStyle
    }
}

# Create results directory | 创建结果目录
New-Item -ItemType Directory -Force -Path $TEST_RESULT_DIR | Out-Null

# Initialize results file | 初始化结果文件
"Test Name,Duration (s),StdDev (s),Peak Memory (KB),Exit Code,File Count,Size Range,Indent Style" | 
    Set-Content -Path "$TEST_RESULT_DIR\format_performance_results.csv"

# Store all results | 存储所有结果
$allResults = @()

Write-Host "Starting format performance tests..."

# 1. Format files with no indentation using different styles
# 使用不同的缩进风格格式化无缩进文件
$noIndentFiles = @("$TEST_CASE_DIR\no_indent1.arxml", "$TEST_CASE_DIR\no_indent2.arxml")

# Test with tab indentation | 使用制表符缩进测试
$result = Measure-ToolPerformance -TestName "no_indent_to_tab" `
    -Command "format -a $TEST_CASE_DIR\no_indent1.arxml -a $TEST_CASE_DIR\no_indent2.arxml -i tab -o $TEST_RESULT_DIR\tab_formatted" `
    -InputFiles $noIndentFiles -IndentStyle "tab"
$allResults += $result

# Test with 2 spaces | 使用2空格测试
$result = Measure-ToolPerformance -TestName "no_indent_to_2space" `
    -Command "format -a $TEST_CASE_DIR\no_indent1.arxml -a $TEST_CASE_DIR\no_indent2.arxml -i 2 -o $TEST_RESULT_DIR\2space_formatted" `
    -InputFiles $noIndentFiles -IndentStyle "2"
$allResults += $result

# Test with 4 spaces | 使用4空格测试
$result = Measure-ToolPerformance -TestName "no_indent_to_4space" `
    -Command "format -a $TEST_CASE_DIR\no_indent1.arxml -a $TEST_CASE_DIR\no_indent2.arxml -i 4 -o $TEST_RESULT_DIR\4space_formatted" `
    -InputFiles $noIndentFiles -IndentStyle "4"
$allResults += $result

# 2. Format mixed indentation files | 格式化混合缩进文件
$mixedFiles = @("$TEST_CASE_DIR\mixed_indent1.arxml", "$TEST_CASE_DIR\mixed_indent2.arxml")
$result = Measure-ToolPerformance -TestName "mixed_indent_normalize" `
    -Command "format -a $TEST_CASE_DIR\mixed_indent1.arxml -a $TEST_CASE_DIR\mixed_indent2.arxml -i 4 -o $TEST_RESULT_DIR\mixed_formatted" `
    -InputFiles $mixedFiles -IndentStyle "4"
$allResults += $result

# 3. Format large files | 格式化大文件
$largeFiles = 1..3 | ForEach-Object { "$TEST_CASE_DIR\large$_.arxml" }
$result = Measure-ToolPerformance -TestName "large_files_format" `
    -Command "format -a $TEST_CASE_DIR\large1.arxml -a $TEST_CASE_DIR\large2.arxml -a $TEST_CASE_DIR\large3.arxml -i 4 -o $TEST_RESULT_DIR\large_formatted" `
    -InputFiles $largeFiles -IndentStyle "4"
$allResults += $result

# 4. Format deep nested files | 格式化深层嵌套文件
$deepFiles = @("$TEST_CASE_DIR\deep1.arxml", "$TEST_CASE_DIR\deep2.arxml")
$result = Measure-ToolPerformance -TestName "deep_nesting_format" `
    -Command "format -a $TEST_CASE_DIR\deep1.arxml -a $TEST_CASE_DIR\deep2.arxml -i 4 -o $TEST_RESULT_DIR\deep_formatted" `
    -InputFiles $deepFiles -IndentStyle "4"
$allResults += $result

# 5. Test sorting with different file sizes | 测试不同文件大小的排序性能
Write-Host "`nTesting sorting performance..."

# Small files with sorting | 小文件排序测试
$result = Measure-ToolPerformance -TestName "small_files_sort_asc" `
    -Command "format -a $TEST_CASE_DIR\no_indent1.arxml -a $TEST_CASE_DIR\no_indent2.arxml -i 4 -s asc -o $TEST_RESULT_DIR\small_sorted_asc" `
    -InputFiles $noIndentFiles -IndentStyle "4"
$allResults += $result

$result = Measure-ToolPerformance -TestName "small_files_sort_desc" `
    -Command "format -a $TEST_CASE_DIR\no_indent1.arxml -a $TEST_CASE_DIR\no_indent2.arxml -i 4 -s desc -o $TEST_RESULT_DIR\small_sorted_desc" `
    -InputFiles $noIndentFiles -IndentStyle "4"
$allResults += $result

# Medium files with sorting | 中等文件排序测试
$result = Measure-ToolPerformance -TestName "mixed_files_sort_asc" `
    -Command "format -a $TEST_CASE_DIR\mixed_indent1.arxml -a $TEST_CASE_DIR\mixed_indent2.arxml -i 4 -s asc -o $TEST_RESULT_DIR\mixed_sorted_asc" `
    -InputFiles $mixedFiles -IndentStyle "4"
$allResults += $result

$result = Measure-ToolPerformance -TestName "mixed_files_sort_desc" `
    -Command "format -a $TEST_CASE_DIR\mixed_indent1.arxml -a $TEST_CASE_DIR\mixed_indent2.arxml -i 4 -s desc -o $TEST_RESULT_DIR\mixed_sorted_desc" `
    -InputFiles $mixedFiles -IndentStyle "4"
$allResults += $result

# Large files with sorting | 大文件排序测试
$result = Measure-ToolPerformance -TestName "large_files_sort_asc" `
    -Command "format -a $TEST_CASE_DIR\large1.arxml -a $TEST_CASE_DIR\large2.arxml -a $TEST_CASE_DIR\large3.arxml -i 4 -s asc -o $TEST_RESULT_DIR\large_sorted_asc" `
    -InputFiles $largeFiles -IndentStyle "4"
$allResults += $result

$result = Measure-ToolPerformance -TestName "large_files_sort_desc" `
    -Command "format -a $TEST_CASE_DIR\large1.arxml -a $TEST_CASE_DIR\large2.arxml -a $TEST_CASE_DIR\large3.arxml -i 4 -s desc -o $TEST_RESULT_DIR\large_sorted_desc" `
    -InputFiles $largeFiles -IndentStyle "4"
$allResults += $result

# Deep nested files with sorting | 深层嵌套文件排序测试
$result = Measure-ToolPerformance -TestName "deep_files_sort_asc" `
    -Command "format -a $TEST_CASE_DIR\deep1.arxml -a $TEST_CASE_DIR\deep2.arxml -i 4 -s asc -o $TEST_RESULT_DIR\deep_sorted_asc" `
    -InputFiles $deepFiles -IndentStyle "4"
$allResults += $result

$result = Measure-ToolPerformance -TestName "deep_files_sort_desc" `
    -Command "format -a $TEST_CASE_DIR\deep1.arxml -a $TEST_CASE_DIR\deep2.arxml -i 4 -s desc -o $TEST_RESULT_DIR\deep_sorted_desc" `
    -InputFiles $deepFiles -IndentStyle "4"
$allResults += $result

# Generate summary report | 生成总结报告
Write-Host "`nGenerating performance summary..."

$summary = @"
Format Performance Test Summary
==============================

Test Results:
------------
$($allResults | ForEach-Object {
    "Test: $($_.TestName)`n" +
    "  Files: $($_.FileCount) files`n" +
    "  Size Range: $($_.SizeRange)`n" +
    "  Indent Style: $($_.IndentStyle)`n" +
    "  Duration: $([Math]::Round($_.Duration, 3)) seconds (±$([Math]::Round($_.DurationStdDev, 3)))`n" +
    "  Peak Memory: $([Math]::Round($_.Memory, 2)) KB`n" +
    "  Exit Code: $($_.ExitCode)`n"
})

Average Metrics:
--------------
Average execution time: $([Math]::Round(($allResults | Measure-Object -Property Duration -Average).Average, 3)) seconds
Average peak memory: $([Math]::Round(($allResults | Measure-Object -Property Memory -Average).Average, 2)) KB
Total files processed: $($allResults | Measure-Object -Property FileCount -Sum).Sum
"@

$summary | Set-Content -Path "$TEST_RESULT_DIR\format_performance_summary.txt"

Write-Host "Performance test completed. Check $TEST_RESULT_DIR\format_performance_results.csv and $TEST_RESULT_DIR\format_performance_summary.txt for details." 