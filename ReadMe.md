# 工程说明
本工程旨在：
1. 读取 Python 整理筛选好的待计算点对
2. 启动多线程进行计算
3. 合并为统一的、方便查询的数组，写入文件，供 Python 程序读取

# 特性1：提供测试脚本比较C++版本的计算输出与MATLAB工具箱FORTRAN版本的输出
## 使用方法
### 1. C++ 侧输出计算结果、准备计算文件
`write2file.cpp` 提供了 `write_param2env` 与 `write_tl2file`。可以在主程序中启用它们。具体位置在 `compute_worker` 函数中。直接搜索即可找到。取消注释即可。

`write_param2env`：将某次计算的参数写入 env 文件，以供MATLAB版本读取

`write_tl2file`：将某次计算的结果写入 txt 文件，默认按照 depth、range、angle排序，以便比较计算差异

这些文件的输出目录都是工作目录，若处于开发环境则为 `build/Release` 。运行C++程序即可获得 env、bty，以及当次计算的计算结果 txt

### 2. MATLAB 侧计算并比较
`runtest_batch.m` 是批次比较版本。具体代码为AI生成，我修改了其中部分寻找文件的逻辑。经过测试，我保证其逻辑正确，但 **注意，我并不保证在你的电脑上能直接运行，可能会有路径错误。** 我没有使用绝对路径，但如果出错了，还请耐心 debug

`test_single_file.m` 是单文件比较版本。你可以先运行这个。

一键运行 `runtest_batch.m` 后，`build/Release/matlab_output` 文件夹下会出现MATLAB版本的计算结果，以及一个比较报告。其中提到了一些文件有差距，但是总体来说误差极小，取接收区块平均值后可以忽略
