cmake_minimum_required(VERSION 3.10)
project(HFTSystem VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 选项
option(BUILD_TESTS "Build tests" ON)
option(BUILD_BENCHMARKS "Build benchmarks" ON)
option(USE_GPU "Use GPU acceleration" OFF)
option(USE_FPGA "Use FPGA acceleration" OFF)
option(BUILD_GUI "Build GUI interface" ON)

# 查找依赖
find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(GTest REQUIRED)
find_package(benchmark REQUIRED)
find_package(TBB REQUIRED)  # Intel Threading Building Blocks

# 如果使用GPU
if(USE_GPU)
    find_package(CUDA REQUIRED)
    add_definitions(-DUSE_GPU)
endif()

# 如果使用FPGA
if(USE_FPGA)
    add_definitions(-DUSE_FPGA)
endif()

# GUI 相关设置
if(BUILD_GUI)
    find_package(Qt5 COMPONENTS Core Widgets Charts REQUIRED)
    add_definitions(-DBUILD_GUI)
    
    # 添加 GUI 源文件
    file(GLOB_RECURSE GUI_SOURCES "gui/*.cpp")
    file(GLOB_RECURSE CLI_SOURCES "cli/*.cpp")
    
    # 添加 Qt MOC 处理
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
endif()

# 包含TensorFlow（如果需要）
# find_package(TensorFlow REQUIRED)

# 头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/execution
    ${CMAKE_CURRENT_SOURCE_DIR}/hardware
    ${CMAKE_CURRENT_SOURCE_DIR}/market
    ${CMAKE_CURRENT_SOURCE_DIR}/network
    ${CMAKE_CURRENT_SOURCE_DIR}/pattern
    ${CMAKE_CURRENT_SOURCE_DIR}/risk
    ${CMAKE_CURRENT_SOURCE_DIR}/strategy
    ${CMAKE_CURRENT_SOURCE_DIR}/synchronization
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# 源文件
file(GLOB_RECURSE CORE_SOURCES "core/*.cpp")
file(GLOB_RECURSE EXECUTION_SOURCES "execution/*.cpp")
file(GLOB_RECURSE HARDWARE_SOURCES "hardware/*.cpp")
file(GLOB_RECURSE MARKET_SOURCES "market/*.cpp")
file(GLOB_RECURSE NETWORK_SOURCES "network/*.cpp")
file(GLOB_RECURSE PATTERN_SOURCES "pattern/*.cpp")
file(GLOB_RECURSE RISK_SOURCES "risk/*.cpp")
file(GLOB_RECURSE STRATEGY_SOURCES "strategy/*.cpp")
file(GLOB_RECURSE SYNCHRONIZATION_SOURCES "synchronization/*.cpp")
file(GLOB_RECURSE UTILS_SOURCES "utils/*.cpp")

# 主程序源文件
set(MAIN_SOURCES main.cpp)

# 合并所有源文件
set(SOURCES
    ${CORE_SOURCES}
    ${EXECUTION_SOURCES}
    ${HARDWARE_SOURCES}
    ${MARKET_SOURCES}
    ${NETWORK_SOURCES}
    ${PATTERN_SOURCES}
    ${RISK_SOURCES}
    ${STRATEGY_SOURCES}
    ${SYNCHRONIZATION_SOURCES}
    ${UTILS_SOURCES}
    ${MAIN_SOURCES}
)

# 创建可执行文件
add_executable(hft_system ${SOURCES})

# 链接依赖
target_link_libraries(hft_system
    Threads::Threads
    nlohmann_json::nlohmann_json
    # TensorFlow::TensorFlow # 如果没有 TensorFlow C++ 包，建议注释掉
)

# 如果使用GPU
if(USE_GPU)
    target_link_libraries(hft_system ${CUDA_LIBRARIES})
endif()

# 如果构建GUI
if(BUILD_GUI)
    target_link_libraries(hft_system
        Qt5::Core
        Qt5::Widgets
        Qt5::Charts
    )
    # 添加GUI源文件
    file(GLOB_RECURSE GUI_SOURCES "gui/*.cpp")
    target_sources(hft_system PRIVATE ${GUI_SOURCES})
endif()

# 安装目标
install(TARGETS hft_system DESTINATION bin)

# 复制配置文件
install(DIRECTORY config/ DESTINATION etc/hft_system)

# 构建测试
if(BUILD_TESTS)
    enable_testing()
    file(GLOB TEST_SOURCES "tests/*.cpp")
    add_executable(hft_system_tests ${TEST_SOURCES})
    target_link_libraries(hft_system_tests
        Threads::Threads
        nlohmann_json::nlohmann_json
    )
    add_test(NAME hft_system_tests COMMAND hft_system_tests)
endif()