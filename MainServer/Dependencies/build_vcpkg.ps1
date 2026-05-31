# 确保输出支持 UTF-8
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

$Host.UI.RawUI.WindowTitle = "vcpkg 编译安装依赖"

$workDir = Split-Path -Parent $MyInvocation.MyCommand.Definition # $PSScriptRoot
$vcpkgDir = Join-Path $workDir "vcpkg"
$vcpkgExe = Join-Path $vcpkgDir "vcpkg.exe"
$env:PATH += ";$vcpkgDir"

$windowsDir = Join-Path $vcpkgDir "installed\x64-windows"

$debugSrc = Join-Path $windowsDir "debug\lib"
$releasesSrc = Join-Path $windowsDir "lib"
$includeSrc = Join-Path $windowsDir "include"
$debugBinSrc = Join-Path $windowsDir "debug\bin"
$releaseBinSrc = Join-Path $windowsDir "bin"

$debugDest = Join-Path $workDir "lib\windows\Debug"
$releaseDest = Join-Path $workDir "lib\windows\Release"
$includeDest = Join-Path $workDir "include\windows"
$debugBinDest = Join-Path $workDir "bin\Debug"
$releaseBinDest = Join-Path $workDir "bin\Release"

# 检查 vcpkg.exe 是否存在
$maxAgeDays = 30
if (Test-Path $vcpkgExe) {
    Write-Host "[INFO] 检测到 vcpkg 已编译完成，跳过克隆与编译" -ForegroundColor Green
    $lastWrite = (Get-Item $vcpkgExe).LastWriteTime

    if (((Get-Date) - $lastWrite).TotalDays -gt $maxAgeDays) {
        Write-Host "[INFO] vcpkg.exe 已存在但过期（$([math]::Round($ageDays.TotalDays, 1)) 天），开始拉取并重新编译" -ForegroundColor Yellow
        git -C $vcpkgDir pull
        & "$vcpkgDir\bootstrap-vcpkg.bat"
    }
    else {
        Write-Host "[INFO] vcpkg.exe 已存在且较新（$([math]::Round($ageDays.TotalDays, 1)) 天），跳过拉取与重新编译" -ForegroundColor Green
    }
}
else {
    Write-Host "[INFO] vcpkg 不存在或未编译，开始克隆并编译..." -ForegroundColor Yellow
    if (-not (git -C $vcpkgDir rev-parse --is-inside-work-tree 2>$null)) {
        git clone https://github.com/Microsoft/vcpkg.git $vcpkgDir
    }
    & "$vcpkgDir\bootstrap-vcpkg.bat"
}

# 安装依赖
vcpkg install asio:x64-windows fmt:x64-windows spdlog:x64-windows zlib:x64-windows

# 清理库文件和头文件
Write-Host "[INFO] ⚠️ 清理库文件和头文件..." -ForegroundColor Blue
if (Test-Path $debugDest) { Remove-Item $debugDest -Recurse -Force }
if (Test-Path $releaseDest) { Remove-Item $releaseDest -Recurse -Force }
if (Test-Path $includeDest) { Remove-Item $includeDest -Recurse -Force }
if (Test-Path $debugBinDest) { Remove-Item $debugBinDest -Recurse -Force }
if (Test-Path $releaseBinDest) { Remove-Item $releaseBinDest -Recurse -Force }
Write-Host "[INFO] ⚠️ 清理库文件和头文件完成" -ForegroundColor Green

# 拷贝库文件和头文件
Write-Host "[INFO] 🔚 拷贝库文件和头文件..." -ForegroundColor Blue
# 定义复制映射表：源文件 → 目标文件
$copyMap = @(
    @{ Src = (Join-Path $debugSrc "zlibd.lib");     Dest = (Join-Path $debugDest "zlib.lib") },
    @{ Src = (Join-Path $releasesSrc "zlib.lib");   Dest = (Join-Path $releaseDest "zlib.lib") },
    @{ Src = (Join-Path $debugBinSrc "zlibd1.dll");     Dest = (Join-Path $debugBinDest "zlibd1.dll") },
    @{ Src = (Join-Path $releaseBinSrc "zlib1.dll");   Dest = (Join-Path $releaseBinDest "zlib1.dll") }
)

# 执行复制操作
foreach ($item in $copyMap) {
    if (Test-Path $item.Src) {
        $destDir = Split-Path $item.Dest
        if (-not (Test-Path $destDir)) {
            New-Item -ItemType Directory -Path $destDir -Force | Out-Null
            Write-Host "📁 创建文件夹: $destDir"
        }

        Copy-Item $item.Src $item.Dest -Force
        Write-Host "复制文件: $($item.Src) → $($item.Dest)"
    } else {
        Write-Warning "源文件不存在: $($item.Src)"
    }
}
Copy-Item $includeSrc $includeDest -Recurse -Force
Write-Host "[INFO] 🔚 拷贝库文件和头文件完成" -ForegroundColor Green

# 倒计时 5 秒退出
for ($i = 5; $i -gt 0; $i--) {
    Write-Host "倒计时 $i 秒退出..." -ForegroundColor Yellow
    Start-Sleep -Seconds 1
}
