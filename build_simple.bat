@echo off
echo Compiling HFT System...

:: 检查 g++ 是否安装
where g++ >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: g++ compiler not found!
    echo Please install MinGW-w64 first.
    exit /b 1
)

:: 创建输出目录
if not exist "bin" mkdir bin

:: 编译
g++ -std=c++17 main.cpp -o bin/HFTSystem.exe

if %errorlevel% neq 0 (
    echo Build failed!
    exit /b 1
)

echo Build successful!
echo You can now run bin\HFTSystem.exe
