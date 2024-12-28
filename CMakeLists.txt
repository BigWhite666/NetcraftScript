cmake_minimum_required(VERSION 3.30)
project(NKScript)

# 在项目定义之后立即设置 Qt 自动处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")

# 设置 Qt5 路径
set(CMAKE_PREFIX_PATH "C:/Qt/5.15.2/msvc2019")
set(Qt5_DIR "C:/Qt/5.15.2/msvc2019/lib/cmake/Qt5")

# 查找 Qt 包
find_package(Qt5 COMPONENTS Widgets REQUIRED)

# 定义项目结构目录
set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PROJECT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PROJECT_EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)

# 明确列出头文件
set(HEADER_FILES
    ${PROJECT_INCLUDE_DIR}/main.h
    ${PROJECT_INCLUDE_DIR}/UI/MainWindow.h
    ${PROJECT_INCLUDE_DIR}/UI/Style/StyleSheet.h
    ${PROJECT_INCLUDE_DIR}/UI/Panels/HomePanel.h
    ${PROJECT_INCLUDE_DIR}/Script/ChatScript.h
    ${PROJECT_INCLUDE_DIR}/Script/MapScript.h
    ${PROJECT_INCLUDE_DIR}/Script/DebugScript.h
    ${PROJECT_INCLUDE_DIR}/Util/WindowHelper.h
    ${PROJECT_INCLUDE_DIR}/UI/Panels/LoginPanel.h
    ${PROJECT_INCLUDE_DIR}/Script/LoginScript.h
    ${PROJECT_INCLUDE_DIR}/Util/InputHelper.h
    ${PROJECT_INCLUDE_DIR}/Util/MessageHandler.h
)

# 明确列出源文件
set(SOURCE_FILES
    main.cpp
    ${PROJECT_SRC_DIR}/dm/dmutils.cpp
    ${PROJECT_SRC_DIR}/UI/MainWindow.cpp
    ${PROJECT_SRC_DIR}/UI/Style/StyleSheet.cpp
    ${PROJECT_SRC_DIR}/UI/Panels/HomePanel.cpp
    ${PROJECT_SRC_DIR}/MemoryRead/Memory.cpp
    ${PROJECT_SRC_DIR}/Script/ChatScript.cpp
    ${PROJECT_SRC_DIR}/Script/MapScript.cpp
    ${PROJECT_SRC_DIR}/Script/DebugScript.cpp
    ${PROJECT_SRC_DIR}/Util/WindowHelper.cpp
    ${PROJECT_SRC_DIR}/UI/Panels/LoginPanel.cpp
    ${PROJECT_SRC_DIR}/Script/LoginScript.cpp
    ${PROJECT_SRC_DIR}/Util/InputHelper.cpp
    ${PROJECT_SRC_DIR}/Util/MessageHandler.cpp
    ${HEADER_FILES}
)

# 在 add_executable 之前添加
if(WIN32)
    # 设置为 Windows 子系统并使用 mainCRTStartup 作为入口点
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

# 修改 add_executable 行
if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES})
else()
    add_executable(${PROJECT_NAME} ${SOURCE_FILES})
endif()

# 设置包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_INCLUDE_DIR}
    ${PROJECT_INCLUDE_DIR}/dm
    ${PROJECT_INCLUDE_DIR}/MemoryRead
    ${PROJECT_INCLUDE_DIR}/Script
    ${PROJECT_INCLUDE_DIR}/UI
    ${PROJECT_INCLUDE_DIR}/UI/Style
    ${PROJECT_INCLUDE_DIR}/UI/Panels
    ${PROJECT_INCLUDE_DIR}/Util
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    ${Qt5Widgets_DEFINITIONS}
    -DWIN32
    # -D_DEBUG
    -D_WINDOWS
    -D_UNICODE
    -DUNICODE
)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Widgets
    ole32 
    oleaut32
    urlmon
    oleacc
)

# 添加 Windows SDK 库路径
target_link_directories(${PROJECT_NAME} PRIVATE
    "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22621.0/um/x86"
)

# 拷贝资源文件 dm.dll
file(COPY ${PROJECT_EXTERNAL_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# 拷贝 Qt DLL
if(WIN32)
    # 复制 Qt 运行时 DLL
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt5::Core>
            $<TARGET_FILE:Qt5::Gui>
            $<TARGET_FILE:Qt5::Widgets>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

    # 创建并复制 platforms 插件
    set(QT_PLUGINS_DIR "${CMAKE_PREFIX_PATH}/plugins")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${QT_PLUGINS_DIR}/platforms"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms"
    )

    # 创建并复制 styles 插件
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/styles
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${QT_PLUGINS_DIR}/styles"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/styles"
    )

    # 创建并复制 imageformats 插件（可能需要）
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/imageformats
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${QT_PLUGINS_DIR}/imageformats"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/imageformats"
    )
endif()

