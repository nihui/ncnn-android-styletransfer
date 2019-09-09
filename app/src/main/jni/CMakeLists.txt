project(styletransferncnn)

cmake_minimum_required(VERSION 3.4.1)

# import ncnn library
add_library(ncnn STATIC IMPORTED)
# change this folder path to yours
set_target_properties(ncnn PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ncnn-android-vulkan-lib/${ANDROID_ABI}/libncnn.a)
include_directories(${CMAKE_SOURCE_DIR}/ncnn-android-vulkan-lib/include/ncnn)

# openmp
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fopenmp")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fomit-frame-pointer -fstrict-aliasing -ffast-math")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fomit-frame-pointer -fstrict-aliasing -ffast-math")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")

# disable rtti and exceptions
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

add_library(styletransferncnn SHARED styletransferncnn_jni.cpp)

target_link_libraries(styletransferncnn
	ncnn
	android
	z
	log
	jnigraphics
	vulkan
)
