cmake_minimum_required( VERSION 3.10 FATAL_ERROR )
set( CMAKE_VERBOSE_MAKEFILE ON )

project( BasicApp )

# 设置 Cinder 路径
get_filename_component( CINDER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE )
get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../" ABSOLUTE )

# 引入 WebSocket++ 头文件路径
include_directories("D:/Study/4 semester/Cinder-master/samples/BasicApp/libs/websocketpp-master/websocketpp")

# 查找 Boost 依赖项（确保安装 Boost）
find_package(Boost REQUIRED COMPONENTS system thread)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    message(STATUS "Boost found: ${Boost_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "Boost not found. Please install Boost.")
endif()

# 添加 DMXPro 相关的源文件
set(SRC_FILES 
    ${APP_PATH}/src/BasicApp.cpp
    ${APP_PATH}/src/DMXPro.cpp
)

# 包含 Cinder
include( "${CINDER_PATH}/proj/cmake/modules/cinderMakeApp.cmake" )

# 构建应用程序
ci_make_app(
    SOURCES     ${SRC_FILES}
    CINDER_PATH ${CINDER_PATH}
)

# 链接 Boost 库
target_link_libraries(BasicApp ${Boost_LIBRARIES})
