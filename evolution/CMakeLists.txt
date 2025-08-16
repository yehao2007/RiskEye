# 🔄 持续优化演示构建配置
cmake_minimum_required(VERSION 3.16)
project(ContinuousOptimizationDemo VERSION 1.0.0 LANGUAGES CXX)

# C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译优化
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
endif()

# 源文件
set(SOURCES
    ContinuousOptimizationDemo.cpp
    ContinuousOptimization.cpp
    ../core/Logger.cpp
    ../core/System.cpp
)

# 头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../core
    ${CMAKE_CURRENT_SOURCE_DIR}/../ai
)

# 创建可执行文件
add_executable(continuous_optimization_demo ${SOURCES})

# 链接库
find_package(Threads REQUIRED)
target_link_libraries(continuous_optimization_demo 
    Threads::Threads
)

# Windows 特定设置
if(WIN32)
    target_link_libraries(continuous_optimization_demo ws2_32)
endif()

# 设置输出目录
set_target_properties(continuous_optimization_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 安装规则
install(TARGETS continuous_optimization_demo
    RUNTIME DESTINATION bin
)
