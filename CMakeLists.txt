cmake_minimum_required(VERSION 3.28)
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
file(GLOB_RECURSE HEADER_FILES
    ${PROJECT_INCLUDE_DIR}/*.h
    ${PROJECT_INCLUDE_DIR}/UI/Components/*.h
    ${PROJECT_INCLUDE_DIR}/UI/Panels/*.h
)

# 明确列出源文件
file(GLOB_RECURSE SOURCE_FILES
    ${PROJECT_SRC_DIR}/*.cpp
    ${PROJECT_SRC_DIR}/UI/Components/*.cpp
    ${PROJECT_SRC_DIR}/UI/Panels/*.cpp
    ${PROJECT_SRC_DIR}/Script/*.cpp
    main.cpp
)

# 在 add_executable 之前添加
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

# 修改 add_executable 行
if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES} ${HEADER_FILES})
else()
    add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
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
    ${PROJECT_INCLUDE_DIR}/UI/Components
    ${PROJECT_INCLUDE_DIR}/Util
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    ${Qt5Widgets_DEFINITIONS}
    -DWIN32
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

    # 创建并复制 imageformats 插件
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/imageformats
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${QT_PLUGINS_DIR}/imageformats"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/imageformats"
    )
endif()

