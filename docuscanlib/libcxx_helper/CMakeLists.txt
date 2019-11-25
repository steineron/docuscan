cmake_minimum_required(VERSION 3.6)

# Enable C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(target docuscanlib)
project(${target} CXX)

# dummy target to bring libc++_shared.so into packages
add_library(opencv_jni_shared SHARED ../native/src/DocuScan.cpp)
