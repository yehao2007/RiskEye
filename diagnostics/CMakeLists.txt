# CMakeLists.txt for HFT Diagnostic System
cmake_minimum_required(VERSION 3.16)
project(HFTDiagnosticSystem VERSION 1.0.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /WX /utf-8)
else()
    add_compile_options(-Wall -Wextra -Werror -O3)
endif()

# 查找依赖包
find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(websocketpp REQUIRED)

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

# 诊断系统核心库
set(DIAGNOSTIC_CORE_SOURCES
    DiagnosticTool.h
    DiagnosticCore.h
    DiagnosticCore.cpp
    DiagnosticPlugin.h
    PluginManager.cpp
    VisualizationAndMonitoring.h
    VisualizationAndMonitoring.cpp
    SystemDiagnostics.h
    SystemDiagnostics.cpp
    EnhancedDiagnostics.h
    EnhancedDiagnostics.cpp
    DiagnosticDataSource.h
    HardwareMonitorPlugin.h
    HardwareMonitorPlugin.cpp
    RemoteMonitor.h
    RemoteMonitor.cpp
    AdvancedFeatures.h
    AdvancedFeatures.cpp
    EnterpriseFeatures.h
    FuturisticFeatures.h
    TradingSpecificFeatures.h
    FinalAdvancedFeatures.h
    SuperDiagnosticSystem.h
    SuperDiagnosticSystem.cpp
    WorldStrongestSystem.h
)

# 创建诊断系统静态库
add_library(hft_diagnostic_core STATIC ${DIAGNOSTIC_CORE_SOURCES})

# 链接依赖
target_link_libraries(hft_diagnostic_core
    PRIVATE
        Threads::Threads
        nlohmann_json::nlohmann_json
        websocketpp::websocketpp
)

# 设置预编译宏
target_compile_definitions(hft_diagnostic_core PRIVATE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    _WIN32_WINNT=0x0601
)

# 示例程序
add_executable(diagnostic_example
    example_usage.cpp
)

target_link_libraries(diagnostic_example
    PRIVATE
        hft_diagnostic_core
        Threads::Threads
        nlohmann_json::nlohmann_json
        websocketpp::websocketpp
)

# 综合功能演示程序
add_executable(comprehensive_demo
    comprehensive_demo.cpp
)

target_link_libraries(comprehensive_demo
    PRIVATE
        hft_diagnostic_core
        Threads::Threads
        nlohmann_json::nlohmann_json
        websocketpp::websocketpp
)

# 世界最强系统 - 终极版本
add_executable(ultimate_system
    ultimate_demo.cpp
    WorldStrongestSystem.cpp
)

target_link_libraries(ultimate_system
    PRIVATE
        hft_diagnostic_core
        Threads::Threads
        nlohmann_json::nlohmann_json
        websocketpp::websocketpp
)

# 安装配置
install(TARGETS hft_diagnostic_core
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES
    DiagnosticTool.h
    DiagnosticCore.h
    DiagnosticPlugin.h
    VisualizationAndMonitoring.h
    SystemDiagnostics.h
    EnhancedDiagnostics.h
    DiagnosticDataSource.h
    HardwareMonitorPlugin.h
    RemoteMonitor.h
    AdvancedFeatures.h
    EnterpriseFeatures.h
    FuturisticFeatures.h
    TradingSpecificFeatures.h
    FinalAdvancedFeatures.h
    SuperDiagnosticSystem.h
    WorldStrongestSystem.h
    DESTINATION include/hft/diagnostics
)

# 创建配置文件
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/HFTDiagnosticConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/HFTDiagnosticConfig.cmake"
    @ONLY
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/HFTDiagnosticConfig.cmake"
    DESTINATION lib/cmake/HFTDiagnostic
)

# 文档生成（如果有Doxygen）
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    
    add_custom_target(doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

# 测试配置
enable_testing()

# 单元测试（如果有Google Test）
find_package(GTest)
if(GTEST_FOUND)
    add_executable(diagnostic_tests
        tests/test_diagnostic_core.cpp
        tests/test_performance_analyzer.cpp
        tests/test_fault_detector.cpp
        tests/test_plugin_manager.cpp
        tests/test_remote_monitor.cpp
    )
    
    target_link_libraries(diagnostic_tests
        PRIVATE
            hft_diagnostic_core
            GTest::GTest
            GTest::Main
            Threads::Threads
    )
    
    add_test(NAME DiagnosticCoreTests COMMAND diagnostic_tests)
endif()

# 打包配置
set(CPACK_PACKAGE_NAME "HFTDiagnosticSystem")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "High-Frequency Trading Diagnostic System")
set(CPACK_PACKAGE_VENDOR "HFT Development Team")
set(CPACK_PACKAGE_CONTACT "support@hft-system.com")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
else()
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
endif()

include(CPack)

# 显示配置信息
message(STATUS "")
message(STATUS "HFT Diagnostic System Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Features:")
message(STATUS "  Real-time monitoring: YES")
message(STATUS "  Remote WebSocket API: YES")
message(STATUS "  Plugin system: YES")
message(STATUS "  Visualization: YES")
message(STATUS "  Alert system: YES")
message(STATUS "  Auto repair: YES")
message(STATUS "  Performance analysis: YES")
message(STATUS "  Hardware monitoring: YES")
message(STATUS "  Network monitoring: YES")
message(STATUS "  Log analysis: YES")
message(STATUS "  Trend prediction: YES")
message(STATUS "")

if(GTEST_FOUND)
    message(STATUS "  Unit tests: YES")
else()
    message(STATUS "  Unit tests: NO (Google Test not found)")
endif()

if(DOXYGEN_FOUND)
    message(STATUS "  Documentation: YES")
else()
    message(STATUS "  Documentation: NO (Doxygen not found)")
endif()

message(STATUS "")
message(STATUS "Usage:")
message(STATUS "  Build: cmake --build .")
message(STATUS "  Install: cmake --install .")
message(STATUS "  Run example: ./diagnostic_example")
message(STATUS "  Generate docs: make doc_doxygen")
message(STATUS "  Run tests: ctest")
message(STATUS "  Create package: cpack")
message(STATUS "")
