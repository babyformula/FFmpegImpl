cmake_minimum_required(VERSION 3.13)
project(FFmpegImpl)

include(cmake/cmake-build-funcs.cmake)

set(CMAKE_CXX_STANDARD 14)

add_executable(FFmpegImpl main.cpp src/FFmpegDecoder.cpp include/FFmpegDecoder.h src/FFmpegH264Encoder.cpp include/FFmpegH264Encoder.h)

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

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h)
find_library(SWSCALE_LIBRARY swscale)

find_path(X264_INCLUDE_DIR x264.h)
find_library(X264_LIBRARY x264)

find_path(X265_INCLUDE_DIR x265.h)
find_library(X265_LIBRARY x265)

find_library(AVFOUNDATION_LIBRARY Avfoundation)

find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)

find_library(BZ2_LIBRARY bz2)

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

message(STATUS "SWSCALE library status:")
message(STATUS "    libraries: ${SWSCALE_LIBRARY}")
message(STATUS "    include path: ${SWSCALE_INCLUDE_DIR}")

message(STATUS "X264 library status:")
message(STATUS "    libraries: ${X264_LIBRARY}")
message(STATUS "    include path: ${X264_INCLUDE_DIR}")

message(STATUS "X265 library status:")
message(STATUS "    libraries: ${X265_LIBRARY}")
message(STATUS "    include path: ${X265_INCLUDE_DIR}")

message(STATUS "AVFOUNDATION library status:")
message(STATUS "    libraries: ${AVFOUNDATION_LIBRARY}")

message(STATUS "AUDIOTOOLBOX library status:")
message(STATUS "    libraries: ${AUDIOTOOLBOX_LIBRARY}")

message(STATUS "BZ2 library status:")
message(STATUS "    libraries: ${BZ2_LIBRARY}")

find_package (Threads)

include_directories(${OpenCV_INCLUDE_DIRS}
        ${AVCODEC_INCLUDE_DIR}
        ${AVFORMAT_INCLUDE_DIR}
        ${AVUTIL_INCLUDE_DIR}
        ${AVDEVICE_INCLUDE_DIR}
        ${SWSCALE_INCLUDE_DIR})

target_link_libraries(FFmpegImpl ${OpenCV_LIBS}
        ${AVCODEC_LIBRARY}
        ${AVFORMAT_LIBRARY}
        ${AVUTIL_LIBRARY}
        ${AVDEVICE_LIBRARY}
        ${SWSCALE_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT})