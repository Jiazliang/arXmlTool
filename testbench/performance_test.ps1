# Performance test script for arXmlTool (PowerShell version)
# 性能测试脚本（PowerShell版本）

# Test directories | 测试目录
$TEST_CASE_DIR = "cases/6.1"
$TEST_RESULT_DIR = "results/6.1"

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
        [int]$Iterations = $TEST_ITERATIONS
    )
    
    Write-Host "`nRunning test: $TestName ($Iterations iterations)"
    Write-Host "Command: $Command"
    
    # Get file info | 获取文件信息
    $fileCount = $InputFiles.Count
    $sizeRange = Get-FileSizeRange $InputFiles
    Write-Host "Files: $fileCount files, Size range: $sizeRange"
    
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
    "$TestName,$([Math]::Round($avgDuration, 3)),$([Math]::Round($durationStdDev, 3)),$([Math]::Round($avgMemory, 2)),$($process.ExitCode),$fileCount,$sizeRange" | 
        Add-Content -Path "$TEST_RESULT_DIR\performance_results.csv"
    
    return @{
        TestName = $TestName
        Duration = $avgDuration
        DurationStdDev = $durationStdDev
        Memory = $avgMemory
        ExitCode = $process.ExitCode
        FileCount = $fileCount
        SizeRange = $sizeRange
    }
}

# Create results directory | 创建结果目录
New-Item -ItemType Directory -Force -Path $TEST_RESULT_DIR | Out-Null

# Initialize results file | 初始化结果文件
"Test Name,Duration (s),StdDev (s),Peak Memory (KB),Exit Code,File Count,Size Range" | 
    Set-Content -Path "$TEST_RESULT_DIR\performance_results.csv"

# Store all results | 存储所有结果
$allResults = @()

Write-Host "Starting performance tests..."

# 1. Small files test (2 files) | 小文件测试（2个文件）
$smallFiles = @("$TEST_CASE_DIR\small1.arxml", "$TEST_CASE_DIR\small2.arxml")
$result = Measure-ToolPerformance -TestName "small_2files" `
    -Command "merge -a $TEST_CASE_DIR\small1.arxml -a $TEST_CASE_DIR\small2.arxml -m $TEST_RESULT_DIR\small_merged.arxml" `
    -InputFiles $smallFiles
$allResults += $result

# 2. Medium files test (5 files) | 中等文件测试（5个文件）
$medFiles = 1..5 | ForEach-Object { "$TEST_CASE_DIR\med$_.arxml" }
$result = Measure-ToolPerformance -TestName "medium_5files" `
    -Command "merge -a $TEST_CASE_DIR\med1.arxml -a $TEST_CASE_DIR\med2.arxml -a $TEST_CASE_DIR\med3.arxml -a $TEST_CASE_DIR\med4.arxml -a $TEST_CASE_DIR\med5.arxml -m $TEST_RESULT_DIR\medium_merged.arxml" `
    -InputFiles $medFiles
$allResults += $result

# 3. Large files test (10 files) | 大文件测试（10个文件）
$largeFiles = 1..10 | ForEach-Object { "$TEST_CASE_DIR\large$_.arxml" }
$result = Measure-ToolPerformance -TestName "large_10files" `
    -Command "merge -a $TEST_CASE_DIR\large1.arxml -a $TEST_CASE_DIR\large2.arxml -a $TEST_CASE_DIR\large3.arxml -a $TEST_CASE_DIR\large4.arxml -a $TEST_CASE_DIR\large5.arxml -a $TEST_CASE_DIR\large6.arxml -a $TEST_CASE_DIR\large7.arxml -a $TEST_CASE_DIR\large8.arxml -a $TEST_CASE_DIR\large9.arxml -a $TEST_CASE_DIR\large10.arxml -m $TEST_RESULT_DIR\large_merged.arxml" `
    -InputFiles $largeFiles
$allResults += $result

# 4. Mixed size test | 混合大小测试
$mixedFiles = @("$TEST_CASE_DIR\small1.arxml", "$TEST_CASE_DIR\med1.arxml", "$TEST_CASE_DIR\large1.arxml")
$result = Measure-ToolPerformance -TestName "mixed_size" `
    -Command "merge -a $TEST_CASE_DIR\small1.arxml -a $TEST_CASE_DIR\med1.arxml -a $TEST_CASE_DIR\large1.arxml -m $TEST_RESULT_DIR\mixed_merged.arxml" `
    -InputFiles $mixedFiles
$allResults += $result

# 5. Deep nesting test | 深层嵌套测试
$deepFiles = @("$TEST_CASE_DIR\deep1.arxml", "$TEST_CASE_DIR\deep2.arxml")
$result = Measure-ToolPerformance -TestName "deep_nesting" `
    -Command "merge -a $TEST_CASE_DIR\deep1.arxml -a $TEST_CASE_DIR\deep2.arxml -m $TEST_RESULT_DIR\deep_merged.arxml" `
    -InputFiles $deepFiles
$allResults += $result

# Generate summary report | 生成总结报告
Write-Host "`nGenerating performance summary..."

$summary = @"
Performance Test Summary
========================

Test Results:
------------
$($allResults | ForEach-Object {
    "Test: $($_.TestName)`n" +
    "  Files: $($_.FileCount) files`n" +
    "  Size Range: $($_.SizeRange)`n" +
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

$summary | Set-Content -Path "$TEST_RESULT_DIR\performance_summary.txt"

Write-Host "Performance test completed. Check $TEST_RESULT_DIR\performance_results.csv and $TEST_RESULT_DIR\performance_summary.txt for details." 