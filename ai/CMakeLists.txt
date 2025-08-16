# 🏆 全球顶级风险预测大模型 - CMake 构建配置
cmake_minimum_required(VERSION 3.18)

project(GlobalRiskPredictionModel
    VERSION 1.0.0
    DESCRIPTION "World-class Risk Prediction Model with Quantum Computing"
    LANGUAGES CXX CUDA
)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 编译器优化标志
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")

# 启用位置无关代码
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ============================================================================
# 依赖项查找
# ============================================================================

# PyTorch
find_package(Torch REQUIRED)
if(NOT Torch_FOUND)
    message(FATAL_ERROR "PyTorch not found. Please install PyTorch C++ API (libtorch)")
endif()

# CUDA (可选)
find_package(CUDA QUIET)
if(CUDA_FOUND)
    enable_language(CUDA)
    message(STATUS "CUDA found: ${CUDA_VERSION}")
    add_definitions(-DUSE_CUDA)
else()
    message(WARNING "CUDA not found. GPU acceleration will be disabled.")
endif()

# OpenMP
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found")
else()
    message(FATAL_ERROR "OpenMP not found")
endif()

# Eigen3
find_package(Eigen3 REQUIRED)
if(Eigen3_FOUND)
    message(STATUS "Eigen3 found")
else()
    message(FATAL_ERROR "Eigen3 not found")
endif()

# nlohmann/json
find_package(nlohmann_json REQUIRED)

# Intel TBB
find_package(TBB REQUIRED)

# Boost
find_package(Boost REQUIRED COMPONENTS system filesystem thread)

# ============================================================================
# 编译器特定设置
# ============================================================================

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math -funroll-loops")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mfma")  # AVX2 和 FMA 支持
    
    # Link Time Optimization for Release builds
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
    endif()
    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math -funroll-loops")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mfma")
    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /Ob2 /Oi /Ot")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# ============================================================================
# 包含目录
# ============================================================================

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${TORCH_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

# ============================================================================
# 源文件
# ============================================================================

set(RISK_PREDICTION_SOURCES
    GlobalRiskPredictionModel.cpp
    QuantumInterface.cpp
    RiskPredictionFactory.cpp
)

set(RISK_PREDICTION_HEADERS
    GlobalRiskPredictionModel.h
    QuantumInterface.h
    RiskPredictionFactory.h
)

# 演示程序源文件
set(DEMO_SOURCES
    GlobalRiskPredictionDemo.cpp
)

# ============================================================================
# 静态库目标
# ============================================================================

add_library(GlobalRiskPrediction STATIC
    ${RISK_PREDICTION_SOURCES}
    ${RISK_PREDICTION_HEADERS}
)

# 设置库属性
set_target_properties(GlobalRiskPrediction PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${RISK_PREDICTION_HEADERS}"
)

# 链接库
target_link_libraries(GlobalRiskPrediction
    PUBLIC
        ${TORCH_LIBRARIES}
        Eigen3::Eigen
        nlohmann_json::nlohmann_json
        TBB::tbb
        OpenMP::OpenMP_CXX
        ${Boost_LIBRARIES}
)

# 如果有CUDA，添加CUDA库
if(CUDA_FOUND)
    target_link_libraries(GlobalRiskPrediction
        PUBLIC
            ${CUDA_LIBRARIES}
            ${CUDA_CUBLAS_LIBRARIES}
            ${CUDA_curand_LIBRARY}
    )
endif()

# 编译选项
target_compile_options(GlobalRiskPrediction
    PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-fPIC>
)

# ============================================================================
# 演示可执行文件
# ============================================================================

add_executable(risk_prediction_demo
    ${DEMO_SOURCES}
)

target_link_libraries(risk_prediction_demo
    PRIVATE
        GlobalRiskPrediction
)

# ============================================================================
# 单元测试 (可选)
# ============================================================================

option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    enable_testing()
    
    # 查找 Google Test
    find_package(GTest QUIET)
    
    if(GTest_FOUND)
        add_executable(risk_prediction_tests
            tests/test_global_risk_prediction.cpp
            tests/test_quantum_interface.cpp
            tests/test_risk_factory.cpp
        )
        
        target_link_libraries(risk_prediction_tests
            PRIVATE
                GlobalRiskPrediction
                GTest::gtest
                GTest::gtest_main
        )
        
        add_test(NAME RiskPredictionTests COMMAND risk_prediction_tests)
        
        message(STATUS "Unit tests enabled")
    else()
        message(WARNING "Google Test not found. Unit tests disabled.")
    endif()
endif()

# ============================================================================
# 基准测试 (可选)
# ============================================================================

option(BUILD_BENCHMARKS "Build benchmarks" ON)

if(BUILD_BENCHMARKS)
    find_package(benchmark QUIET)
    
    if(benchmark_FOUND)
        add_executable(risk_prediction_benchmark
            benchmarks/benchmark_prediction_speed.cpp
            benchmarks/benchmark_accuracy.cpp
        )
        
        target_link_libraries(risk_prediction_benchmark
            PRIVATE
                GlobalRiskPrediction
                benchmark::benchmark
        )
        
        message(STATUS "Benchmarks enabled")
    else()
        message(WARNING "Google Benchmark not found. Benchmarks disabled.")
    endif()
endif()

# ============================================================================
# Python 绑定 (可选)
# ============================================================================

option(BUILD_PYTHON_BINDINGS "Build Python bindings" OFF)

if(BUILD_PYTHON_BINDINGS)
    find_package(pybind11 QUIET)
    
    if(pybind11_FOUND)
        pybind11_add_module(py_risk_prediction
            python_bindings/risk_prediction_bindings.cpp
        )
        
        target_link_libraries(py_risk_prediction
            PRIVATE
                GlobalRiskPrediction
        )
        
        message(STATUS "Python bindings enabled")
    else()
        message(WARNING "pybind11 not found. Python bindings disabled.")
    endif()
endif()

# ============================================================================
# 安装配置
# ============================================================================

# 安装库
install(TARGETS GlobalRiskPrediction
    EXPORT GlobalRiskPredictionTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/hft/ai
)

# 安装头文件
install(FILES ${RISK_PREDICTION_HEADERS}
    DESTINATION include/hft/ai
)

# 安装演示程序
install(TARGETS risk_prediction_demo
    DESTINATION bin
)

# 导出目标
install(EXPORT GlobalRiskPredictionTargets
    FILE GlobalRiskPredictionTargets.cmake
    NAMESPACE HFT::
    DESTINATION lib/cmake/GlobalRiskPrediction
)

# 创建配置文件
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/GlobalRiskPredictionConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/GlobalRiskPredictionConfig.cmake"
    INSTALL_DESTINATION lib/cmake/GlobalRiskPrediction
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/GlobalRiskPredictionConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/GlobalRiskPredictionConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/GlobalRiskPredictionConfigVersion.cmake"
    DESTINATION lib/cmake/GlobalRiskPrediction
)

# ============================================================================
# 文档生成 (可选)
# ============================================================================

option(BUILD_DOCS "Build documentation" OFF)

if(BUILD_DOCS)
    find_package(Doxygen QUIET)
    
    if(Doxygen_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        
        add_custom_target(docs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
        
        message(STATUS "Documentation generation enabled")
    else()
        message(WARNING "Doxygen not found. Documentation disabled.")
    endif()
endif()

# ============================================================================
# 性能分析支持
# ============================================================================

option(ENABLE_PROFILING "Enable profiling support" OFF)

if(ENABLE_PROFILING)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
        message(STATUS "Profiling support enabled (gprof)")
    endif()
endif()

# ============================================================================
# 内存检查支持
# ============================================================================

option(ENABLE_SANITIZERS "Enable sanitizers" OFF)

if(ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(SANITIZER_FLAGS "-fsanitize=address,undefined -fno-omit-frame-pointer")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
        message(STATUS "Sanitizers enabled (AddressSanitizer, UndefinedBehaviorSanitizer)")
    endif()
endif()

# ============================================================================
# 状态报告
# ============================================================================

message(STATUS "")
message(STATUS "=== Global Risk Prediction Model Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "PyTorch version: ${Torch_VERSION}")
message(STATUS "CUDA support: ${CUDA_FOUND}")
message(STATUS "OpenMP support: ${OpenMP_CXX_FOUND}")
message(STATUS "TBB support: ${TBB_FOUND}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "Build benchmarks: ${BUILD_BENCHMARKS}")
message(STATUS "Build Python bindings: ${BUILD_PYTHON_BINDINGS}")
message(STATUS "Build documentation: ${BUILD_DOCS}")
message(STATUS "Enable profiling: ${ENABLE_PROFILING}")
message(STATUS "Enable sanitizers: ${ENABLE_SANITIZERS}")
message(STATUS "==================================================")
message(STATUS "")

# ============================================================================
# 自定义目标
# ============================================================================

# 清理目标
add_custom_target(clean-all
    COMMAND ${CMAKE_BUILD_TOOL} clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeCache.txt
)

# 格式化代码目标
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i -style=file ${RISK_PREDICTION_SOURCES} ${RISK_PREDICTION_HEADERS} ${DEMO_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formatting source code"
    )
endif()

# 静态分析目标
find_program(CLANG_TIDY clang-tidy)
if(CLANG_TIDY)
    add_custom_target(static-analysis
        COMMAND ${CLANG_TIDY} ${RISK_PREDICTION_SOURCES} -- -I${CMAKE_CURRENT_SOURCE_DIR} ${TORCH_INCLUDE_DIRS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running static analysis"
    )
endif()
