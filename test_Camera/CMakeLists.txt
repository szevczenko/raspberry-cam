cmake_minimum_required (VERSION 2.8) 
project (raspicam_test)
set(raspicam_DIR "/usr/local/lib/cmake")
find_package(raspicam REQUIRED)
find_package(OpenCV)
#IF  ( OpenCV_FOUND AND raspicam_CV_FOUND)
#MESSAGE(STATUS "COMPILING OPENCV TESTS")
add_executable (test test.cpp)  
target_link_libraries (test ${raspicam_LIBS})
#ELSE()
#MESSAGE(FATAL_ERROR "OPENCV NOT FOUND IN YOUR SYSTEM")
#ENDIF()