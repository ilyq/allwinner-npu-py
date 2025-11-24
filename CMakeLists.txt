cmake_minimum_required(VERSION 3.10)
project(allwinner-npu)

# 检测是否正在进行交叉编译
message(STATUS "Cross-compiling for Android")

# Android/Termux 配置
set(PYTHON_INCLUDE_DIR "/home/sd1/work/allwinner-npu-py/3rd_party/termux-python/python3.12")
set(PYTHON_LIBRARY "/home/sd1/work/allwinner-npu-py/3rd_party/termux-python/libpython3.12.so.1.0")

# 设置 Android 工具链
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/3rd_party/android-ndk-r25c/build/cmake/android.toolchain.cmake")
set(ANDROID_ABI "arm64-v8a")
set(ANDROID_PLATFORM "android-24")


# 完全禁用 pybind11 的自动查找
set(PYBIND11_NOPYTHON ON)

add_subdirectory(3rd_party/pybind11)

# 检查路径是否存在
if(NOT EXISTS ${PYTHON_INCLUDE_DIR})
    message(FATAL_ERROR "Python include directory not found: ${PYTHON_INCLUDE_DIR}")
endif()

if(NOT EXISTS ${PYTHON_LIBRARY})
    message(FATAL_ERROR "Python library not found: ${PYTHON_LIBRARY}")
endif()

# 手动创建模块
add_library(allwinner_npu SHARED src/allwinner_npu.cpp
    3rd_party/npuruntime/npulib.cpp
    3rd_party/npuruntime/npu_util.cpp
)

# 设置包含目录 
target_include_directories(allwinner_npu PRIVATE 
    ${PYTHON_INCLUDE_DIR}
    ${pybind11_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/3rd_party/npuruntime/include
    ${PROJECT_SOURCE_DIR}/3rd_party/npuruntime
)

# 设置链接库
target_link_libraries(allwinner_npu PRIVATE ${PYTHON_LIBRARY}
    ${PROJECT_SOURCE_DIR}/3rd_party/npuruntime/lib_android/A733/arm64-v8a/libVIPhal.so
    ${PROJECT_SOURCE_DIR}/3rd_party/npuruntime/lib_android/A733/arm64-v8a/libNBGlinker.so
    ${PROJECT_SOURCE_DIR}/3rd_party/npuruntime/lib_android/compat/arm64-v8a/libVIPliteCompat.so
    ${PROJECT_SOURCE_DIR}/3rd_party/npuruntime/lib_android/libc++_shared.so
    ${PROJECT_SOURCE_DIR}/3rd_party/npuruntime/lib_android/libomp.so
)

# 设置编译选项
target_compile_definitions(allwinner_npu PRIVATE 
    VERSION_INFO="${EXAMPLE_VERSION_INFO}"
    PYBIND11_PYTHON_VERSION=3.12
)

# 设置 C++ 标准
set_target_properties(allwinner_npu PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

# 设置输出文件名
set_target_properties(allwinner_npu PROPERTIES
    PREFIX ""
    SUFFIX ".so"
)