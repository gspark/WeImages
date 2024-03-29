cmake_minimum_required(VERSION 3.21)
project(WeImages LANGUAGES CXX C VERSION 0.0.8.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
set(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")

#SET(CMAKE_BUILD_TYPE Release)
#SET(CMAKE_BUILD_TYPE Debug)

# add_definitions(-D_AMD64_)

set(OpenCV_DIR d:\\ops\\opencv)
set(OpenCV_STATIC ON)
find_package(OpenCV REQUIRED)
#添加头文件
include_directories(${OpenCV_INCLUDE_DIRS})

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /FORCE:MULTIPLE" CACHE INTERNAL "" FORCE)
    # ignore failed
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:\"zlib.lib;zlibd.lib\"" CACHE INTERNAL "" FORCE)
endif()

#if(MSVC)
#    add_compile_options(
#        $<$<CONFIG:>:/MT> #---------|
#        $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
#        $<$<CONFIG:Release>:/MT> #--|
#    )
#endif()

if (("${CMAKE_BUILD_TYPE}" STREQUAL "Debug") OR ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo") )
    message("build type: " ${CMAKE_BUILD_TYPE})
    add_definitions(-D_DEBUG)
    set(QT_STATIC_PATH "d:/ops/qt-6.5.0/static")
    ## SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    ## SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    # Debug用DLL运行库，Release用静态链接运行库
    ## set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:DebugDLL>")
else ()
    message("build type: " "Release")
    set(QT_STATIC_PATH "d:/ops/qt-6.5.0/static")
    # Debug、Release都用静态链接运行库，推荐用于不想让用户装各种VC++运行库的软件
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

list(APPEND CMAKE_PREFIX_PATH ${QT_STATIC_PATH})

#定义变量
## set(PREBUILT_LIB_DIRECTORY D:/ops/msys64/mingw64/lib)

include_directories(${QT_STATIC_PATH}/include
        ${QT_STATIC_PATH}/include/QtCore
        ${QT_STATIC_PATH}/include/QtWidgets
        ${QT_STATIC_PATH}/include/QtGui)

set_target_properties(${TARGET_NAME} PROPERTIES DEBUG_POSTFIX "d")
set_target_properties(${TARGET_NAME} PROPERTIES RELEASE_POSTFIX "")

# 设置自动生成moc文件
set(CMAKE_AUTOMOC ON)
# 设置自动处理.qrc文件
set(CMAKE_AUTORCC ON)
# 设置自动生成ui.h文件
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Svg)
find_package(Qt6 COMPONENTS Widgets)
find_package(Qt6 COMPONENTS Concurrent)

# add_definitions(-DVERSION)

file(GLOB_RECURSE all_src_file
        src/*.cpp src/*.hpp src/*.c src/*.h src/*.ui
        )

SET(RCC_FILES resources/images/app.qrc)

if(MSVC)
    set(MY_VERSIONINFO_RC "${PROJECT_BINARY_DIR}/versionInfo.rc")
    configure_file("resources/version/versionInfo.rc.in" "${MY_VERSIONINFO_RC}")
endif()

add_executable(${PROJECT_NAME}
        ${all_src_file}
        ${RCC_FILES}
#        resources/icons/app.rc
        ${MY_VERSIONINFO_RC}
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE FALSE
        )

#link_directories(${CMAKE_CACHEFILE_DIR}/lib)

target_link_libraries(${PROJECT_NAME} PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Svg
        Qt::Widgets
        Qt::Concurrent
        opencv_world
        )

#set(ZLIB_INCLUDE_DIR "d:/ops/zlib/include")
#set(ZLIB_LIBRARY "d:/ops/zlib/lib/zlibstatic.lib")
#find_package(ZLIB REQUIRED)
#if (ZLIB_FOUND AND ("${CMAKE_BUILD_TYPE}" STREQUAL "Release"))
#    message(STATUS "zlib lib = ${ZLIB_LIBRARIES}")
#    #    target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES})
#    # 链接 dll.a 有依赖dll
#    target_link_libraries(${PROJECT_NAME} PUBLIC ${PREBUILT_LIB_DIRECTORY}/libz.a)
#    target_link_libraries(${PROJECT_NAME} PUBLIC ${PREBUILT_LIB_DIRECTORY}/libzstd.a)
#endif ()

#链接gcc-g++静态库
if (MINGW AND ("${CMAKE_BUILD_TYPE}" STREQUAL "Release"))
    message(STATUS "is mingw")
    #    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS} -static-libgcc -s")
    #    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS} -static-libgcc -static-libstdc++ -s")
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
    #    target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS} -lpthread)
endif ()

