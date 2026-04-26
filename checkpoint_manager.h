#pragma once

#include <string>
#include <mutex>
#include <fstream>
#include <iostream>
#include <cstring>

// 全局数据结构声明（在 underwater_acoustic_computing_core.cpp 中定义）
extern double ans[101][101][101][11];
extern bool visited[101][101][101][11];
extern bool written[101][101][101][11];

class CheckpointManager
{
private:
    std::mutex write_mutex_;
    std::string checkpoint_file_ = "checkpoint.dat";
    std::string results_file_ = "average_TL_results.txt";

public:
    CheckpointManager() = default;

    /**
     * @brief 从 checkpoint.dat 读取 visited 数组状态
     * @return true 如果成功读取，false 如果文件不存在或读取失败
     */
    bool load_checkpoint();

    /**
     * @brief 将 visited 数组保存到 checkpoint.dat
     * @return true 如果成功保存
     */
    bool save_checkpoint();

    /**
     * @brief 从 average_TL_results.txt 读取所有已计算结果
     * 这个方法会：
     * 1. 逐行解析 txt 文件
     * 2. 将结果值加载到 ans 数组
     * 3. 更新 visited 数组为 true（表示已计算）
     * 4. 更新 written 数组为 true（表示已写入）
     * @return 读取的行数
     */
    int load_results();

    /**
     * @brief 定时扫描新完成的计算，将新结果追加到文件
     * 扫描所有 visited[i][j][k][l] == true && written[i][j][k][l] == false 的结果
     * 将其追加到 average_TL_results.txt，并标记为已写入
     * @return 新写入的结果数
     */
    int periodic_write();

    /**
     * @brief 追加单条结果到文件
     * @param enemy_y Y 坐标（已分组）
     * @param uuv_x X 坐标（已分组）
     * @param uuv_y Y 坐标（已分组）
     * @param uuv_z Z 坐标（已分组）
     * @param average_TL 平均 TL 值
     * @return true 如果成功
     */
    bool append_result(int enemy_y, int uuv_x, int uuv_y, int uuv_z, double average_TL);

    /**
     * @brief 计算已计算的结果数
     */
    int count_visited() const;

    /**
     * @brief 计算已写入的结果数
     */
    int count_written() const;

    /**
     * @brief 计算总共需要计算的结果数
     */
    static constexpr int total_results() { return 101 * 101 * 101 * 11; }
};
