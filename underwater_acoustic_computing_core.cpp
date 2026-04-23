#include <iostream>
#include <atomic>
#include <cstring>
#include <iomanip>
#include <chrono>
#include <thread>
#include <atomic>

// 这一行必须添加，Bellhop C++规定的，然后把 bellhopcxxlib.dll 放在可执行文件同目录下
#define BHC_DLL_IMPORT 1

#include "debug_command.h"
#include "utils.h"
#include "bellhop_params.h"
#include "write2file.h"
#include "config_param.h"

// 最终计算结果数组
// [enemy_y][uuv_x][uuv_y][uuv_z] -> average TL
double ans[101][101][101][11];
// 计算总对数
int total_pairs = 0;
// 全局进度计数器
std::atomic<int> global_computed_count(0);

// 定义控制台输出回调函数，用于捕获引擎内部的日志
void OutputCallback(const char* message)
{
        // std::cout << "Out: " << message << std::endl << std::flush;
}
void PrtCallback(const char* message)
{
        // std::cout << message << std::flush;
}

// 定义原子变量以监控计算是否完成
std::atomic<bool> going;
void CompletedCallback()
{
        going = false;
}

void compute_worker(int start, int end, std::vector<PointPair>& pairs)
{
        INFO("Thread started: processing pairs " << start << " to " << end);
        // 每个线程独立加载自己的模板参数和输出结构体
        // 实例化参数、输出和初始化结构体
        bhc::bhcParams<true> params;
        bhc::bhcOutputs<true, true> outputs;
        bhc::bhcInit init;
        // 加载模板 env, 初始化本线程计算单例
        init.FileRoot = "qianshui";
        init.outputCallback = OutputCallback;
        init.prtCallback = PrtCallback;
        init.completedCallback = CompletedCallback;
        bhc::setup(init, params, outputs);

        // 计算指定范围内的点对
        for (auto it = start; it < end && it < pairs.size(); ++it) {
                auto& pair = pairs[it];
                
                // 首先，计算当前 pair 点对的 Bellhop 参数
                pair.generate_key();
                pair.compute_self_bellhop_parameters();

                config_bhc_params(params, pair);

                // 替换完毕，直接开始计算！
                bhc::extsetup_blocking(params, true);
                bhc::run(params, outputs);

                // 由于是阻塞计算，此时应该已经计算完成，开始读取计算结果
                TLField tlField;
                compute_all_TL(params, outputs, tlField);

                // 计算平均 TL, 存储到 ans 数组中
                double average_TL = compute_average_TL(tlField);
                int enemy_y = static_cast<int>(pair.receiver.y);
                int uuv_x = static_cast<int>(pair.source.x);
                int uuv_y = static_cast<int>(pair.source.y);
                int uuv_z = static_cast<int>(pair.source.z);
                ans[enemy_y / 100][uuv_x / 100][uuv_y / 100][uuv_z / 50] = average_TL;

                // 将结果输出到文件
                write_tl2file(pair, tlField);

                // 输出到文件以供MATLAB计算
                write_tl2file(pair, tlField);

                global_computed_count.fetch_add(1, std::memory_order_relaxed);
        }

        // 将计算结果写出到文件中，通常会生成 qianshui.shd (声场文件) 或相关输出
        // std::cout << "4. Writing output..." << std::endl;
        // bhc::writeout(params, outputs, "qianshui");

        // 清理分配的内存和释放引擎资源
        bhc::finalize(params, outputs);

        SUCCESS("Thread completed: processed pairs " << start << " to " << end);
}

int main()
{
        // 获取本机建议的线程数
        int num_threads = std::thread::hardware_concurrency();
        num_threads = 6; // 如果是单例，bellhop自己似乎也能启用多线程功能。说实话我也不太明白究竟应该设多少。这个有待后人实验吧
        INFO("Using " << num_threads << " threads for computation.");
        std::vector<std::thread> threads;

        // 读取 npz 文件，获取待计算的点对
        std::vector<PointPair> key_pairs;
        try {
                key_pairs = loadKeyPairsFromNPZ();
                total_pairs = key_pairs.size();
                std::cout << "Total key pairs loaded: " << key_pairs.size() << std::endl;
        }
        catch (const std::exception& e) {
                std::cerr << "Error loading NPZ: " << e.what() << std::endl;
                return 1;
        }
        std::cout << "Press Enter to start computation..." << std::endl;
        getchar(); // 暂停以查看输出

        // 分配计算任务
        INFO("Computing tasks are being allocated to threads...");
        total_pairs = 20; // 先测试用，后续再放开
        int points_per_thread = total_pairs / num_threads;
        for (int i = 0; i < num_threads; ++i) {
                int start = i * points_per_thread;
                int end = (i == num_threads - 1) ? total_pairs : (i + 1) * points_per_thread;
                threads.emplace_back(compute_worker, start, end, std::ref(key_pairs));
        }

        // 等待计算完成，顺便提供性能监视器
        auto last_time = std::chrono::high_resolution_clock::now();
        auto now_time = std::chrono::high_resolution_clock::now();
        auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(now_time - last_time);
        double it_per_sec = 0.0;
        int last_done = 0;
        double estimated_total_time = 0.0;
        while (true) {
                now_time = std::chrono::high_resolution_clock::now();
                elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now_time - last_time);

                int done = global_computed_count.load(std::memory_order_relaxed);

                it_per_sec = (done - last_done) / (elapsed.count() / 1000.0) * 2;       // *2 是因为这个线程500ms刷新一次
                last_done = done;
                last_time = now_time;
                estimated_total_time = (total_pairs - done) / double(it_per_sec);

                std::cout << "\rProcessing: " << std::fixed << std::setprecision(2)
                        << (done / static_cast<double>(total_pairs) * 100.0) << "% ("
                        << done << "/" << total_pairs << ")"
                        << " | it per sec: " << it_per_sec << "s"
                        << " | Estimated total time: " << estimated_total_time << "s  "
                        << estimated_total_time / 60 << "m  "
                        << estimated_total_time / 60 / 60 << "h  "
                        << estimated_total_time / 60 / 60 / 24 << "d"
                        << std::flush;

                if (done >= total_pairs)
                        break;

                std::this_thread::sleep_for(
                        std::chrono::milliseconds(500));
        }

        // 等待所有线程完成
        for (auto& thread : threads) {
                if (thread.joinable()) {
                        thread.join();
                }
        }

        return 0;
}