<#
.SYNOPSIS
    同步 Source/ 目录的文件到 _SharedCode.vcxproj（Pre-build 自动调用）

.DESCRIPTION
    扫描 Source/ 下的 .cpp/.c/.h/.hpp 文件，与 _SharedCode.vcxproj 中的
    ClCompile / ClInclude 条目对比，自动新增或移除。
    仅操作 Source\ 内的条目，不影响 JUCE 模块和 JuceLibraryCode 条目。

.PARAMETER ProjectDir
    项目根目录（包含 Source/ 的目录）。默认为脚本所在目录的父目录。
#>

param(
    [string]$ProjectDir = (Split-Path -Parent $PSScriptRoot)
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# 查找 .jucer 获取构建目录
$jucerFile = Get-ChildItem -Path $ProjectDir -Filter "*.jucer" -File | Select-Object -First 1
if (-not $jucerFile) {
    Write-Host "[!] 未找到 .jucer 文件，跳过源文件同步" -ForegroundColor Yellow
    exit 0
}

[xml]$jucer = Get-Content -Path $jucerFile.FullName -Encoding UTF8

# 查找 VS 导出格式
$exportFormats = $jucer.JUCERPROJECT.EXPORTFORMATS
$vsNode = $null
foreach ($ver in @("VS2026", "VS2022", "VS2019", "VS2017")) {
    $node = $exportFormats.SelectSingleNode($ver)
    if ($node) { $vsNode = $node; break }
}

if (-not $vsNode) {
    Write-Host "[!] 未找到 Visual Studio 导出格式，跳过" -ForegroundColor Yellow
    exit 0
}

$targetFolder = $vsNode.targetFolder
$buildDir = Join-Path $ProjectDir $targetFolder

$sharedCodeVcx = Get-ChildItem -Path $buildDir -Filter "*_SharedCode.vcxproj" -File -ErrorAction SilentlyContinue |
    Select-Object -First 1

if (-not $sharedCodeVcx) {
    Write-Host "[!] 未找到 *_SharedCode.vcxproj，跳过源文件同步" -ForegroundColor Yellow
    exit 0
}

$sourceDir = Join-Path $ProjectDir "Source"

# 1) 扫描实际文件系统
$actualCompile = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
$actualInclude = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)

Get-ChildItem -Path $sourceDir -Recurse -File -ErrorAction SilentlyContinue | ForEach-Object {
    $ext = $_.Extension.ToLower()
    $relPath = $_.FullName.Substring($ProjectDir.TrimEnd('\').Length).TrimStart('\') -replace '/', '\'
    $vcxRel  = "..\..\$relPath"

    switch ($ext) {
        { $_ -in '.cpp', '.c' }  { [void]$actualCompile.Add($vcxRel) }
        { $_ -in '.h', '.hpp' }  { [void]$actualInclude.Add($vcxRel) }
    }
}

# 2) 读取 vcxproj 文本
$vcxText = [System.IO.File]::ReadAllText($sharedCodeVcx.FullName)

# 3) 提取现有 Source\ 条目
$existCompile = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
$existInclude = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)

[regex]::Matches($vcxText, '<ClCompile\s+Include="(\.\.\\\.\.\\Source\\[^"]+)"') | ForEach-Object {
    [void]$existCompile.Add($_.Groups[1].Value)
}
[regex]::Matches($vcxText, '<ClInclude\s+Include="(\.\.\\\.\.\\Source\\[^"]+)"') | ForEach-Object {
    [void]$existInclude.Add($_.Groups[1].Value)
}

# 4) 计算差异
$addCompile    = @($actualCompile | Where-Object { -not $existCompile.Contains($_) })
$removeCompile = @($existCompile  | Where-Object { -not $actualCompile.Contains($_) })
$addInclude    = @($actualInclude | Where-Object { -not $existInclude.Contains($_) })
$removeInclude = @($existInclude  | Where-Object { -not $actualInclude.Contains($_) })

# 5) 执行移除 (删除整行，仅匹配自闭合标签)
foreach ($path in ($removeCompile + $removeInclude)) {
    $escaped = [regex]::Escape($path)
    $vcxText = $vcxText -replace "(?m)^\s*<Cl(?:Compile|Include)\s+Include=`"$escaped`"\s*/>\s*\r?\n", ""
}

# 6) 执行新增 — 插入到最后一个 Source\ 条目之后
if ($addCompile.Count -gt 0) {
    $newLines = ($addCompile | Sort-Object | ForEach-Object { "    <ClCompile Include=`"$_`"/>" }) -join "`r`n"
    $lastMatch = [regex]::Matches($vcxText, '(?m)^(\s*<ClCompile\s+Include="\.\.\\\.\.\\Source\\[^"]+"\s*/>)')
    if ($lastMatch.Count -gt 0) {
        $anchor = $lastMatch[$lastMatch.Count - 1].Value
        $vcxText = $vcxText.Replace($anchor, "$anchor`r`n$newLines")
    }
    else {
        $vcxText = $vcxText -replace '(?m)(^\s*<ItemGroup>\s*\r?\n\s*<ClCompile)', "$newLines`r`n`$1"
    }
}

if ($addInclude.Count -gt 0) {
    $newLines = ($addInclude | Sort-Object | ForEach-Object { "    <ClInclude Include=`"$_`"/>" }) -join "`r`n"
    $lastMatch = [regex]::Matches($vcxText, '(?m)^(\s*<ClInclude\s+Include="\.\.\\\.\.\\Source\\[^"]+"\s*/>)')
    if ($lastMatch.Count -gt 0) {
        $anchor = $lastMatch[$lastMatch.Count - 1].Value
        $vcxText = $vcxText.Replace($anchor, "$anchor`r`n$newLines")
    }
    else {
        $vcxText = $vcxText -replace '(?m)(^\s*<ItemGroup>\s*\r?\n\s*<ClInclude)', "$newLines`r`n`$1"
    }
}

# 7) 写回并报告
$totalChanges = $addCompile.Count + $removeCompile.Count + $addInclude.Count + $removeInclude.Count
if ($totalChanges -gt 0) {
    [System.IO.File]::WriteAllText($sharedCodeVcx.FullName, $vcxText)
    Write-Host "[+] vcxproj 源文件同步: +$($addCompile.Count + $addInclude.Count) 新增, -$($removeCompile.Count + $removeInclude.Count) 移除" -ForegroundColor Green
    foreach ($p in $addCompile)    { Write-Host "      + $p" -ForegroundColor DarkGreen }
    foreach ($p in $addInclude)    { Write-Host "      + $p" -ForegroundColor DarkGreen }
    foreach ($p in $removeCompile) { Write-Host "      - $p" -ForegroundColor DarkRed }
    foreach ($p in $removeInclude) { Write-Host "      - $p" -ForegroundColor DarkRed }
}
else {
    Write-Host "[=] vcxproj 源文件列表已是最新" -ForegroundColor Green
}
