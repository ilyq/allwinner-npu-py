python3 -c "import sysconfig; import json; print(json.dumps(sysconfig.get_paths(), indent=2))"
{
"stdlib": "/Users/dong/.pyenv/versions/3.10.7/lib/python3.10",
"platstdlib": "/Users/dong/worker/pypro/allwinner-npu-py/.venv/lib/python3.10",
"purelib": "/Users/dong/worker/pypro/allwinner-npu-py/.venv/lib/python3.10/site-packages",
"platlib": "/Users/dong/worker/pypro/allwinner-npu-py/.venv/lib/python3.10/site-packages",
"include": "/Users/dong/.pyenv/versions/3.10.7/include/python3.10",
"platinclude": "/Users/dong/.pyenv/versions/3.10.7/include/python3.10",
"scripts": "/Users/dong/worker/pypro/allwinner-npu-py/.venv/bin",
"data": "/Users/dong/worker/pypro/allwinner-npu-py/.venv"
}

Python3_EXECUTABLE（解释器路径）
python -c "import sys; print(sys.executable)"

Python3_INCLUDE_DIRS（头文件路径）
python -c "import sysconfig; print(sysconfig.get_path('include'))"

Python3_LIBRARIES（库文件路径）
python -c "import sysconfig; print(sysconfig.get_config_var('LIBDIR'))"

clangd 配置语法提示和代码跳转

```
# 生成编译数据库
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# 创建符号链接到项目根目录
ln -sf build/compile_commands.json .
```

Android NDK 下载

```
https://github.com/android/ndk/wiki/Unsupported-Downloads
```

# CMakeLists.txt 配置定义顺序

```
# 1. 基础设置
cmake_minimum_required(VERSION 3.10)
project(allwinner-npu)

# 2. 平台检测和消息输出
message(STATUS "Cross-compiling for Android")

# 3. 工具链和平台设置
set(CMAKE_TOOLCHAIN_FILE "...")
set(ANDROID_ABI "arm64-v8a")
set(ANDROID_PLATFORM "android-24")

# 4. 路径设置
set(PYTHON_INCLUDE_DIR "...")
set(PYTHON_LIBRARY "...")

# 5. 路径验证（在创建目标前检查）
if(NOT EXISTS ${PYTHON_INCLUDE_DIR})
    message(FATAL_ERROR "...")
endif()

# 6. 第三方库配置
set(PYBIND11_NOPYTHON ON)
add_subdirectory(3rd_party/pybind11)

# 7. 创建目标 (C++代码)
add_library(allwinner_npu SHARED ...)

# 8. 目标配置
target_include_directories(...) (头文件)
target_link_libraries(...) (链接库, *.os)
target_compile_definitions(...)(可选)

# 9. 目标属性设置(可选)
set_target_properties(...)
```

## add_library 顺序

```
# 开发阶段优化：常改动的文件在前
add_library(allwinner_npu SHARED
    src/main.cpp                    # 经常修改的接口文件
    src/new_feature.cpp             # 正在开发的功能
    3rd_party/npuruntime/npulib.cpp    # 稳定的第三方代码
    3rd_party/npuruntime/npu_util.cpp  # 稳定的工具函数
)


# 逻辑分组排序
add_library(allwinner_npu SHARED
    # 1. 主入口和接口文件
    src/main.cpp
    src/interface.cpp

    # 2. 核心功能实现
    src/core/npu_wrapper.cpp
    src/core/processor.cpp

    # 3. 工具函数
    src/utils/logger.cpp
    src/utils/converter.cpp

    # 4. 第三方集成
    3rd_party/npuruntime/npulib.cpp
    3rd_party/npuruntime/npu_util.cpp
)
```

## target_include_directories

- 目录顺序很重要，会影响头文件的查找顺序

```
# 策略：系统库 → 第三方库 → 项目代码
target_include_directories(allwinner_npu PRIVATE
    # 1. 语言和系统库
    ${PYTHON_INCLUDE_DIR}

    # 2. 主要第三方库
    ${pybind11_INCLUDE_DIR}
    ${OTHER_LIB_INCLUDE_DIRS}

    # 3. 项目公共接口
    ${PROJECT_SOURCE_DIR}/include

    # 4. 项目实现细节（谨慎使用）
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/3rd_party/npuruntime
)
```
