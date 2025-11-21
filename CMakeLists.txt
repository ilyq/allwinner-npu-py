cmake_minimum_required(VERSION 3.10)
project(allwinner-npu)

add_subdirectory(3rd_party/pybind11)

# 禁止 pybind11 查找系统 Python
set(PYBIND11_FINDPYTHON OFF CACHE BOOL "Disable automatic find_package(Python)")
# 手动指定 Python3.10

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")  # macOS
    set(Python3_EXECUTABLE "/Users/dong/worker/pypro/allwinner-npu-py/.venv/bin/python3.10")
    set(Python3_INCLUDE_DIRS /Users/dong/.pyenv/versions/3.10.7/include/python3.10)
    set(Python3_LIBRARIES /Users/dong/.pyenv/versions/3.10.7/lib)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")  # Linux
    set(Python3_EXECUTABLE /home/sd1/work/allwinner-npu-py/.venv/bin/python)
    set(Python3_INCLUDE_DIRS /home/sd1/.local/share/uv/python/cpython-3.10.18-linux-x86_64-gnu/include/python3.10)
    set(Python3_LIBRARIES /home/sd1/.local/share/uv/python/cpython-3.10.18-linux-x86_64-gnu/lib)
else()
    message(FATAL_ERROR "Unsupported operating system: ${CMAKE_SYSTEM_NAME}")
endif()

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

pybind11_add_module(allwinner_npu src/main.cpp)

target_compile_definitions(allwinner_npu PRIVATE VERSION_INFO="${EXAMPLE_VERSION_INFO}")
