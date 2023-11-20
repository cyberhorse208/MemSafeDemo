# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)

# Declares and names the project.

project("nativetest")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        nativetest

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp ubsan-demo.cpp)

#编译executable
#为了顺利执行native程序，hello-mte编译完成后，要从build/intermediates/cmake/debug/obj/arm64-v8a/目录下拷贝到src/main/jniLibs目录
#并改名为hello-mte.so
#并且AndroidManifest.xml中Application下要配置android:extractNativeLibs="true"
#然后在安装app时，才会解压到/data/app/~~~~~xxx~~~/lib/arm64目录下
#然后就可以通过Runtime.getRuntime().exec(getApplicationInfo().nativeLibraryDir+"/hello-mte.so")的方式来执行了
add_executable(hello-mte hello-mte.cpp use_mte.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

# https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
target_compile_options(nativetest
        PUBLIC
        -g -fsanitize=undefined,integer -fno-omit-frame-pointer
        -fno-sanitize-recover=all
        )
set_target_properties(nativetest PROPERTIES LINK_FLAGS -fsanitize=undefined,integer)

target_link_libraries( # Specifies the target library.
        nativetest

        # Links the target library to the log library
        # included in the NDK.

        ${log-lib})

target_link_libraries(hello-mte ${log-lib})