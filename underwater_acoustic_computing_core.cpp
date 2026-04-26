#include <iostream>
#include <atomic>
#include <cstring>
#include <iomanip>
#include <chrono>
#include <thread>
#include <atomic>
#include <fstream>
#include <vector>
#include <string>

// 这一行必须添加，Bellhop C++规定的，然后把 bellhopcxxlib.dll 放在可执行文件同目录下
#define BHC_DLL_IMPORT 1

#include "debug_command.h"
#include "utils.h"
#include "bellhop_params.h"
#include "write2file.h"
#include "config_param.h"
#include "checkpoint_manager.h"

// 最终计算结果数组
// [enemy_y][uuv_x][uuv_y][uuv_z] -> average TL
double ans[101][101][101][11];
// 已计算标记数组
bool visited[101][101][101][11];
// 已写入文件标记数组
bool written[101][101][101][11];
// 计算总对数
int total_pairs = 0;
// 全局进度计数器
std::atomic<int> global_computed_count(0);
// 后台写入线程停止信号
std::atomic<bool> should_stop_writing(false);

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
                
                int idx_y = enemy_y / 100;
                int idx_x = uuv_x / 100;
                int idx_y2 = uuv_y / 100;
                int idx_z = uuv_z / 50;

                // 只有在未计算过时才计算和存储
                if (!visited[idx_y][idx_x][idx_y2][idx_z]) {
                        ans[idx_y][idx_x][idx_y2][idx_z] = average_TL;
                        visited[idx_y][idx_x][idx_y2][idx_z] = true;  // 标记为已计算
                }

                // 将结果输出到文件
                // write_tl2file(pair, tlField);

                // 输出到文件以供MATLAB计算
                // write_param2env(pair, params);

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
        // ========== 启动恢复逻辑 ==========
        CheckpointManager checkpoint;
        
        // 如果存在 checkpoint.dat，恢复已计算的标记
        checkpoint.load_checkpoint();
        
        // 读取 average_TL_results.txt，恢复已计算的结果值到 ans 数组
        int recovered_results = checkpoint.load_results();
        
        std::cout << "Recovered " << recovered_results << " previously computed results." << std::endl;
        int recovered_count = checkpoint.count_visited();
        std::cout << "Total marked as computed: " << recovered_count << " out of " 
                  << checkpoint.total_results() << std::endl;
        
        // ========== 开始计算 ==========
        // 获取本机建议的线程数
        int num_threads = std::thread::hardware_concurrency();
        num_threads = 4; // 如果是单例，bellhop自己似乎也能启用多线程功能。说实话我也不太明白究竟应该设多少。这个有待后人实验吧
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
        // std::cout << "Press Enter to start computation..." << std::endl;
        // getchar(); // 暂停以查看输出

        // 分配计算任务
        INFO("Computing tasks are being allocated to threads...");
        // total_pairs = 20; // 测试用
        int points_per_thread = total_pairs / num_threads;
        for (int i = 0; i < num_threads; ++i) {
                int start = i * points_per_thread;
                int end = (i == num_threads - 1) ? total_pairs : (i + 1) * points_per_thread;
                threads.emplace_back(compute_worker, start, end, std::ref(key_pairs));
        }

        // ========== 启动后台写入线程 ==========
        std::thread write_thread([&checkpoint]() {
                while (!should_stop_writing) {
                        std::this_thread::sleep_for(std::chrono::seconds(5));
                        if (!should_stop_writing) {
                                int written = checkpoint.periodic_write();
                                if (written > 0) {
                                        INFO("Background write: " << written << " new results written to file.");
                                }
                        }
                }
        });

        // 等待计算完成，顺便提供性能监视器
        auto start_time = std::chrono::high_resolution_clock::now();
        auto last_time = std::chrono::high_resolution_clock::now();
        auto now_time = std::chrono::high_resolution_clock::now();
        auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(now_time - last_time);
        auto pasted =
                std::chrono::duration_cast<std::chrono::milliseconds>(now_time - start_time);
        double it_per_sec = 0.0;
        int last_done = 0;
        double estimated_total_time = 0.0;
        while (true) {
                now_time = std::chrono::high_resolution_clock::now();
                pasted = std::chrono::duration_cast<std::chrono::milliseconds>(now_time - start_time);

                int done = global_computed_count.load(std::memory_order_relaxed);

                it_per_sec = done / (pasted.count() / 1000.0);

                estimated_total_time = (total_pairs - done) / double(it_per_sec);

                std::cout << "\rProcessing: " << std::fixed << std::setprecision(2)
                        << (done / static_cast<double>(total_pairs) * 100.0) << "% ("
                        << done << "/" << total_pairs << ")"
                        << " | it per sec: " << it_per_sec << "it/s"
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

        // ========== 计算完成，进行最后处理 ==========
        // 停止后台写入线程
        should_stop_writing = true;
        write_thread.join();

        // 确保所有新结果都写入了
        int final_write = checkpoint.periodic_write();
        if (final_write > 0) {
                std::cout << "Final write: " << final_write << " remaining results written to file." << std::endl;
        }

        // 保存最终的 checkpoint 状态
        checkpoint.save_checkpoint();

        std::cout << "\nComputation completed successfully!" << std::endl;
        std::cout << "Total results saved: " << checkpoint.count_written() << " out of " 
                  << checkpoint.total_results() << std::endl;
        std::cout << "Results file: average_TL_results.txt" << std::endl;
        std::cout << "Checkpoint file: checkpoint.dat" << std::endl;

        return 0;
}