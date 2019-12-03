cmake_minimum_required(VERSION 3.6)

# Enable C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(target docuscanlib)
project(${target} CXX)
include_directories(../native/include)
# dummy target to bring libc++_shared.so into packages
add_library(docuscan_jni_shared SHARED ../native/src/DocuScan.cpp)


#set_target_properties(docuscan_jni_shared PROPERTIES
#        IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/libbar.so"
#        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/include/libbar"
#        )
set(ANDROID_OPENCV_COMPONENTS "opencv_java" CACHE STRING "")

message(STATUS "Building with ANDROID_ABI=${ANDROID_ABI}")
#find_package(OpenCV REQUIRED COMPONENTS ${ANDROID_OPENCV_COMPONENTS})
#
#file(GLOB srcs *.cpp *.c)
#file(GLOB hdrs *.hpp *.h)
#
#include_directories("${CMAKE_CURRENT_LIST_DIR}")
#add_library(${target} SHARED ${srcs} ${hdrs})

#// is this needed?
#target_link_libraries(docuscan_jni_shared ${ANDROID_OPENCV_COMPONENTS})

#find_library( # Sets the name of the path variable.
#        opencv
#
#        # Specifies the name of the NDK library that
#        # you want CMake to locate.
#        opencv_java4 )
#target_link_libraries( # Specifies the target library.
#        docuscan_jni_shared
#
#        # Links the target library to the log library
#        # included in the NDK.
#        ${opencv} )

target_link_libraries( docuscan_jni_shared
        ${PROJECT_SOURCE_DIR}/../src/main/jniLibs/${ANDROID_ABI}/libopencv_java4.so )