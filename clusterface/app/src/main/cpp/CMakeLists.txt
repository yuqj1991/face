# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        clusterFacelib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        cluserFace.cpp
        v4l2core.c
        #src/main/cpp/mjpeg_decoder.c
        mjpg_decode2.c)

include_directories(/home/deepano/workspace/clusterFace/app/libs/libjpeg-turbo-1.5.0/include)
add_library(jpeg-turbo1500 SHARED IMPORTED)
set_target_properties(jpeg-turbo1500 PROPERTIES IMPORTED_LOCATION /home/deepano/workspace/clusterFace/app/libs/libjpeg-turbo-1.5.0/lib/${ANDROID_ABI}/libjpeg-turbo1500.so)

include_directories(/home/deepano/workspace/clusterFace/app/libs/libFVFace/include)
add_library(FVFace SHARED IMPORTED)
set_target_properties(FVFace PROPERTIES IMPORTED_LOCATION /home/deepano/workspace/clusterFace/app/libs/libFVFace/lib/${ANDROID_ABI}/libFVFace.so)


# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log
        android
        jnigraphics)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        clusterFacelib

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        android
        jpeg-turbo1500
        FVFace
        jnigraphics)