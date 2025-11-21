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