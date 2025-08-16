# 应用程序打包配置
set(APP_NAME "HFTSystem")
set(APP_VERSION "1.0.0")
set(COMPANY_NAME "HFT")

# 生成版本信息文件
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.h"
)

# Windows 资源文件
if(WIN32)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/app.rc"
    )
    set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.ico")
endif()

# 添加可执行文件
add_executable(${APP_NAME} WIN32
    ${SOURCES}
    ${GUI_SOURCES}
    ${CLI_SOURCES}
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_BINARY_DIR}/app.rc>
)

# 设置属性
set_target_properties(${APP_NAME} PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 安装配置
install(TARGETS ${APP_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

# 复制依赖项
if(WIN32)
    # Windows 依赖项
    get_target_property(QT_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")
    
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            $<TARGET_FILE:Qt5::Core>
            $<TARGET_FILE:Qt5::Gui>
            $<TARGET_FILE:Qt5::Widgets>
            $<TARGET_FILE:Qt5::Charts>
            $<TARGET_FILE_DIR:${APP_NAME}>
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            --no-compiler-runtime
            --no-translations
            --no-system-d3d-compiler
            $<TARGET_FILE:${APP_NAME}>
    )
endif()
