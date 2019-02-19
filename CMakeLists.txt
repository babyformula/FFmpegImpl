cmake_minimum_required(VERSION 3.13)
project(FFmpegImpl)

include(cmake/cmake-build-funcs.cmake)

set(CMAKE_CXX_STANDARD 14)

if(NOT CMAKE_BUILD_TYPE)
    # default to Release build for GCC builds
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

if(BUILD_SHARED_LIBRARIES)
  set(FFMMPEG_LIBTYPE SHARED)
else()
  set(FFMMPEG_LIBTYPE STATIC)
endif()

FFMMPEG_OPTION(BUILD_SHARED_LIBRARIES       "Build shared library"               OFF)

include_directories("/Users/spectrum/caffe-augmentation/build/include") 
link_directories("/Users/spectrum/caffe-augmentation/build/lib")

find_package(OpenCV REQUIRED)
message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

set(EXT_LIBS
        ${OpenCV_LIBS}
        )

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY avutil)

find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h)
find_library(AVDEVICE_LIBRARY avdevice)

find_path(AVFILTER_INCLUDE_DIR libavfilter/avfilter.h)
find_library(AVFILTER_LIBRARY avfilter)

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h)
find_library(SWSCALE_LIBRARY swscale)

find_path(X264_INCLUDE_DIR x264.h)
find_library(X264_LIBRARY x264)

find_path(X265_INCLUDE_DIR x265.h)
find_library(X265_LIBRARY x265)

message(STATUS "AVCODEC library status:")
message(STATUS "    libraries: ${AVCODEC_LIBRARY}")
message(STATUS "    include path: ${AVCODEC_INCLUDE_DIR}")

message(STATUS "AVFORMAT library status:")
message(STATUS "    libraries: ${AVFORMAT_LIBRARY}")
message(STATUS "    include path: ${AVFORMAT_INCLUDE_DIR}")

message(STATUS "AVUTIL library status:")
message(STATUS "    libraries: ${AVUTIL_LIBRARY}")
message(STATUS "    include path: ${AVUTIL_INCLUDE_DIR}")

message(STATUS "AVDEVICE library status:")
message(STATUS "    libraries: ${AVDEVICE_LIBRARY}")
message(STATUS "    include path: ${AVDEVICE_INCLUDE_DIR}")

message(STATUS "AVFILTER library status:")
message(STATUS "    libraries: ${AVFILTER_LIBRARY}")
message(STATUS "    include path: ${AVFILTER_INCLUDE_DIR}")

message(STATUS "SWSCALE library status:")
message(STATUS "    libraries: ${SWSCALE_LIBRARY}")
message(STATUS "    include path: ${SWSCALE_INCLUDE_DIR}")

message(STATUS "X264 library status:")
message(STATUS "    libraries: ${X264_LIBRARY}")
message(STATUS "    include path: ${X264_INCLUDE_DIR}")

message(STATUS "X265 library status:")
message(STATUS "    libraries: ${X265_LIBRARY}")
message(STATUS "    include path: ${X265_INCLUDE_DIR}")

include_directories("./include/")

file(GLOB_RECURSE FFMMPEG_ALL_HEADERS "include/*.h" "include/*.hpp")
file(GLOB_RECURSE FFMMPEG_ALL_SRCS "src/*.cpp" "src/*.cc" "src/*.c")

ffmmpeg_make_group("${FFMMPEG_ALL_HEADERS}")
ffmmpeg_make_group("${FFMMPEG_ALL_SRCS}")

set(libname "ffmmpeg")
set(FFMMPEG_ALL_LIBS ${libname})
ffmmpeg_buildlibrary(${libname} ${FFMMPEG_LIBTYPE} SOURCES ${FFMMPEG_ALL_HEADERS} ${FFMMPEG_ALL_SRCS} DEPENDENCIES ${THIRD_PARTY_DEPENDENCIES})

install(FILES ${FFMMPEG_ALL_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/)

list(APPEND THIRD_PARTY_DEPENDENCIES "ffmmpeg")

find_package (Threads)
list(APPEND THIRD_PARTY_DEPENDENCIES ${OpenCV_LIBS})
list(APPEND THIRD_PARTY_DEPENDENCIES ${AVCODEC_LIBRARY})
list(APPEND THIRD_PARTY_DEPENDENCIES ${AVFORMAT_LIBRARY})
list(APPEND THIRD_PARTY_DEPENDENCIES ${AVUTIL_LIBRARY})
list(APPEND THIRD_PARTY_DEPENDENCIES ${AVDEVICE_LIBRARY})
list(APPEND THIRD_PARTY_DEPENDENCIES ${AVFILTER_LIBRARY})
list(APPEND THIRD_PARTY_DEPENDENCIES ${SWSCALE_LIBRARY})
list(APPEND THIRD_PARTY_DEPENDENCIES ${CMAKE_THREAD_LIBS_INIT})

include_directories(${OpenCV_INCLUDE_DIRS}
        ${AVCODEC_INCLUDE_DIR}
        ${AVFORMAT_INCLUDE_DIR}
        ${AVUTIL_INCLUDE_DIR}
        ${AVDEVICE_INCLUDE_DIR}
        ${AVFILTER_INCLUDE_DIR}
        ${SWSCALE_INCLUDE_DIR})

ffmmpeg_subdirlist(test_subdirs "test")
foreach(subdir ${test_subdirs})
  if(EXISTS "${subdir}/CMakeLists.txt")
	add_subdirectory(${subdir})
  endif()
endforeach()