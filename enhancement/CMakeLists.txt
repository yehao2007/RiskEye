cmake_minimum_required(VERSION 3.18)
project(HFT_SystemEnhancement VERSION 2.0.0 LANGUAGES CXX CUDA)

# 🎯 全方面提升项目配置
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)

# 🚀 高性能编译选项
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native -flto -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -fsanitize=address -fsanitize=undefined")

# 📊 性能分析选项
if(ENABLE_PROFILING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -fprofile-arcs -ftest-coverage")
endif()

# 🎯 优化目标定义
add_compile_definitions(
    HFT_ENHANCEMENT_ENABLED=1
    TARGET_LATENCY_NS=15000
    TARGET_THROUGHPUT_OPS=1000000
    ENABLE_QUANTUM_COMPUTING=1
    ENABLE_AI_ACCELERATION=1
    ENABLE_HARDWARE_OPTIMIZATION=1
)

# 📦 依赖包查找
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem thread)
find_package(PkgConfig REQUIRED)
find_package(OpenMP REQUIRED)

# PyTorch C++ (libtorch)
find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

# CUDA (for GPU acceleration)
find_package(CUDAToolkit REQUIRED)
enable_language(CUDA)

# Intel TBB (for parallelization)
find_package(TBB REQUIRED)

# Intel MKL (for optimized math)
find_package(MKL CONFIG)

# OpenCV (for image processing in alternative data)
find_package(OpenCV REQUIRED)

# Eigen3 (for linear algebra)
find_package(Eigen3 REQUIRED)

# Protocol Buffers (for serialization)
find_package(Protobuf REQUIRED)

# gRPC (for microservices)
find_package(gRPC REQUIRED)

# 🏗️ 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/enhancement
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/ai
    ${CMAKE_SOURCE_DIR}/trading
    ${CMAKE_SOURCE_DIR}/risk
    ${CMAKE_SOURCE_DIR}/execution
    ${CMAKE_SOURCE_DIR}/market
    ${CMAKE_SOURCE_DIR}/strategy
    ${CMAKE_SOURCE_DIR}/monitoring
    ${CMAKE_SOURCE_DIR}/hardware
    ${CMAKE_SOURCE_DIR}/network
    ${CMAKE_SOURCE_DIR}/utils
    ${Boost_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)

# 🎯 核心增强模块库
add_library(hft_enhancement SHARED
    # 增强框架核心
    enhancement/SystemEnhancementPlan.cpp
    
    # 核心架构增强
    enhancement/CoreArchitectureEnhancer.cpp
    enhancement/EventLoopOptimizer.cpp
    enhancement/MemoryPoolManager.cpp
    enhancement/ThreadPoolManager.cpp
    
    # 性能引擎增强
    enhancement/PerformanceEngineEnhancer.cpp
    enhancement/LatencyOptimizer.cpp
    enhancement/ThroughputOptimizer.cpp
    enhancement/CPUOptimizer.cpp
    enhancement/MemoryOptimizer.cpp
    
    # AI智能化增强
    enhancement/AIIntelligenceEnhancer.cpp
    enhancement/ReinforcementLearningEngine.cpp
    enhancement/OnlineLearningSystem.cpp
    enhancement/ModelOptimizer.cpp
    
    # 风险管理增强
    enhancement/RiskManagementEnhancer.cpp
    enhancement/RealTimeRiskMonitor.cpp
    enhancement/PredictiveRiskAnalyzer.cpp
    enhancement/StressTestEngine.cpp
    enhancement/ComplianceManager.cpp
    
    # 执行优化增强
    enhancement/ExecutionOptimizationEnhancer.cpp
    enhancement/SmartOrderRouter.cpp
    enhancement/OrderSlicingEngine.cpp
    enhancement/ExecutionAlgorithmManager.cpp
    enhancement/LiquidityOptimizer.cpp
    
    # 市场数据引擎增强
    enhancement/MarketDataEngineEnhancer.cpp
    enhancement/DataIngestionEngine.cpp
    enhancement/RealTimeProcessor.cpp
    enhancement/SentimentAnalyzer.cpp
    enhancement/DataDistributor.cpp
    
    # 策略框架增强
    enhancement/StrategyFrameworkEnhancer.cpp
    enhancement/DynamicStrategyManager.cpp
    enhancement/GeneticOptimizer.cpp
    enhancement/PortfolioOptimizer.cpp
    enhancement/AdvancedBacktester.cpp
    
    # 监控系统增强
    enhancement/MonitoringSystemEnhancer.cpp
    enhancement/SystemHealthMonitor.cpp
    enhancement/AnomalyDetector.cpp
    enhancement/MetricsCollector.cpp
    enhancement/VisualizationEngine.cpp
    
    # 硬件加速增强
    enhancement/HardwareAccelerationEnhancer.cpp
    enhancement/GPUAccelerator.cpp
    enhancement/FPGAAccelerator.cpp
    enhancement/NetworkOptimizer.cpp
    enhancement/QuantumAccelerator.cpp
    
    # 网络优化增强
    enhancement/NetworkOptimizationEnhancer.cpp
    enhancement/LowLatencyNetworkStack.cpp
    enhancement/KernelBypassEngine.cpp
    enhancement/ConnectionManager.cpp
    enhancement/BandwidthOptimizer.cpp
)

# 🎯 链接依赖库
target_link_libraries(hft_enhancement
    ${TORCH_LIBRARIES}
    ${OpenCV_LIBS}
    ${Boost_LIBRARIES}
    Threads::Threads
    OpenMP::OpenMP_CXX
    TBB::tbb
    Eigen3::Eigen
    protobuf::libprotobuf
    gRPC::grpc++
    CUDA::cudart
    CUDA::curand
    CUDA::cublas
    CUDA::cufft
)

# Intel MKL链接（如果找到）
if(MKL_FOUND)
    target_link_libraries(hft_enhancement ${MKL_LIBRARIES})
    target_compile_definitions(hft_enhancement PRIVATE USE_MKL=1)
endif()

# 🚀 全面提升演示程序
add_executable(enhancement_demo
    enhancement/EnhancementDemo.cpp
    core/System.cpp
    core/Logger.cpp
    core/Configuration.cpp
    ai/GlobalRiskPredictionModel.cpp
    ai/ProfessionalConfigGenerator.cpp
)

target_link_libraries(enhancement_demo
    hft_enhancement
    ${TORCH_LIBRARIES}
    ${OpenCV_LIBS}
    ${Boost_LIBRARIES}
    Threads::Threads
)

# 🧪 配置向导程序
add_executable(config_wizard
    ai/ConfigWizard.cpp
    ai/ProfessionalConfigGenerator.cpp
    core/Logger.cpp
)

target_link_libraries(config_wizard
    hft_enhancement
    ${Boost_LIBRARIES}
)

# 📊 性能基准测试
add_executable(performance_benchmark
    tests/PerformanceBenchmark.cpp
    enhancement/SystemEnhancementPlan.cpp
    core/System.cpp
    core/Logger.cpp
)

target_link_libraries(performance_benchmark
    hft_enhancement
    ${TORCH_LIBRARIES}
    ${Boost_LIBRARIES}
    Threads::Threads
    OpenMP::OpenMP_CXX
)

# 🎯 CUDA内核编译
if(CUDA_FOUND)
    set_property(TARGET hft_enhancement PROPERTY CUDA_SEPARABLE_COMPILATION ON)
    
    # GPU加速内核
    add_library(hft_gpu_kernels STATIC
        enhancement/gpu/RiskCalculationKernels.cu
        enhancement/gpu/MatrixOperationKernels.cu
        enhancement/gpu/OptimizationKernels.cu
        enhancement/gpu/MLInferenceKernels.cu
    )
    
    set_target_properties(hft_gpu_kernels PROPERTIES CUDA_RESOLVE_DEVICE_SYMBOLS ON)
    target_link_libraries(hft_enhancement hft_gpu_kernels)
endif()

# 🔧 编译器特定优化
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(hft_enhancement PRIVATE
        -mavx2
        -mfma
        -funroll-loops
        -fprefetch-loop-arrays
        -ffast-math
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(hft_enhancement PRIVATE
        -mavx2
        -mfma
        -funroll-loops
        -ffast-math
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(hft_enhancement PRIVATE
        /arch:AVX2
        /fp:fast
        /Oi
        /Ot
    )
endif()

# 📦 安装配置
install(TARGETS hft_enhancement enhancement_demo config_wizard performance_benchmark
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY enhancement/ DESTINATION include/hft/enhancement
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# 📋 打包配置
set(CPACK_PACKAGE_NAME "HFT-SystemEnhancement")
set(CPACK_PACKAGE_VERSION "2.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "世界顶级高频交易系统全方面提升包")
set(CPACK_PACKAGE_VENDOR "HFT Enhancement Team")
set(CPACK_PACKAGE_CONTACT "support@hft-enhancement.com")

include(CPack)

# 🧪 测试配置
enable_testing()

add_test(NAME enhancement_unit_tests
    COMMAND ${CMAKE_BINARY_DIR}/tests/unit_tests
)

add_test(NAME performance_benchmark_test
    COMMAND ${CMAKE_BINARY_DIR}/performance_benchmark --quick-test
)

add_test(NAME integration_test
    COMMAND ${CMAKE_BINARY_DIR}/enhancement_demo --automated-test
)

# 📊 性能报告生成
add_custom_target(performance_report
    COMMAND ${CMAKE_BINARY_DIR}/performance_benchmark --full-report
    COMMAND ${CMAKE_BINARY_DIR}/enhancement_demo --generate-report
    DEPENDS performance_benchmark enhancement_demo
    COMMENT "Generating comprehensive performance reports"
)

# 🔄 持续集成支持
add_custom_target(ci_build
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --config Release
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    COMMENT "Continuous Integration Build and Test"
)

# 📈 代码覆盖率报告
if(ENABLE_COVERAGE)
    find_program(GCOV_PATH gcov)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(GCOV_PATH AND LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' --output-file coverage.info
            COMMAND ${GENHTML_PATH} coverage.info --output-directory coverage_report
            COMMENT "Generating code coverage report"
        )
    endif()
endif()

# 📝 文档生成
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_SOURCE_DIR}/docs/Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(docs
        ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
    )
endif()

# 🎯 构建总结
message(STATUS "=== HFT System Enhancement Configuration ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "CUDA Support: ${CUDA_FOUND}")
message(STATUS "PyTorch Support: ${Torch_FOUND}")
message(STATUS "OpenCV Support: ${OpenCV_FOUND}")
message(STATUS "Intel MKL: ${MKL_FOUND}")
message(STATUS "OpenMP Support: ${OpenMP_FOUND}")
message(STATUS "==========================================")

# 🚀 快速启动目标
add_custom_target(quick_start
    COMMAND ${CMAKE_BINARY_DIR}/config_wizard
    COMMAND ${CMAKE_BINARY_DIR}/enhancement_demo
    DEPENDS config_wizard enhancement_demo
    COMMENT "Quick start: Configuration wizard and enhancement demo"
)
