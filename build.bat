@echo off

:: 创建构建目录
if not exist build mkdir build
cd build

:: 编译Verilog代码（假设使用Xilinx Vivado）
echo Compiling photonic_order_engine.v...
vivado -mode batch -source ../scripts/synth.tcl

:: 编译C++代码
 echo Compiling C++ code...
cl.exe /EHsc /O2 /arch:AVX512 ^
    ../hardware/PhotonicEngine.cpp ^
    ../strategy/VectorizedStrategyEngine.cpp ^
    ../risk/AdvancedRiskManager.cpp ^
    ../utils/LowLatencyLogger.cpp ^
    ../main.cpp ^
    /Fe:hft_system.exe

:: 检查编译是否成功
if %ERRORLEVEL% EQU 0 (
    echo Build successful! Executable: build/hft_system.exe
) else (
    echo Build failed!
)

cd ..
