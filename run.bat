@echo off

:: 加载FPGA比特流（假设使用Xilinx Vivado）
echo Loading FPGA bitstream...
vivado -mode batch -source ../scripts/program.tcl

:: 运行高频交易系统
echo Starting HFT system...
build\hft_system.exe