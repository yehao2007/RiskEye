@echo off
echo ===================================================
echo      自运营系统演示构建脚本
echo ===================================================

echo 创建构建目录...
if not exist build mkdir build
cd build

echo 编译自运营系统演示程序...
cl /EHsc /std:c++17 /I.. /Fe:SelfOperatingSystemDemo.exe ^
  ..\diagnostics\SelfOperatingSystemDemo.cpp ^
  ..\core\SelfOperatingSystem.cpp

if %ERRORLEVEL% NEQ 0 (
  echo 编译失败，错误代码: %ERRORLEVEL%
  exit /b %ERRORLEVEL%
)

echo 编译成功，运行演示程序...
SelfOperatingSystemDemo.exe

cd ..
echo ===================================================
echo                  演示结束
echo ===================================================
