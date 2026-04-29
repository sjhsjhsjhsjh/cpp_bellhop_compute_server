#include "checkpoint_manager.h"
#include <sstream>
#include <iomanip>
#include <vector>
#include <tuple>

bool CheckpointManager::load_checkpoint()
{
    std::ifstream file(checkpoint_file_, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Checkpoint file not found. Starting fresh computation." << std::endl;
        return false;
    }

    // 读取 visited 数组
    file.read(reinterpret_cast<char*>(visited), sizeof(visited));
    
    if (!file.good()) {
        std::cerr << "Error reading checkpoint file." << std::endl;
        return false;
    }

    file.close();
    std::cout << "Checkpoint loaded successfully. " 
              << count_visited() << " results marked as computed." << std::endl;
    return true;
}

bool CheckpointManager::save_checkpoint()
{
    std::ofstream file(checkpoint_file_, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open checkpoint file for writing." << std::endl;
        return false;
    }

    // 写入 visited 数组
    file.write(reinterpret_cast<const char*>(visited), sizeof(visited));
    
    if (!file.good()) {
        std::cerr << "Error writing checkpoint file." << std::endl;
        return false;
    }

    file.close();
    std::cout << "Checkpoint saved successfully." << std::endl;
    return true;
}

int CheckpointManager::load_results()
{
    std::ifstream file(results_file_);
    if (!file.is_open()) {
        std::cout << "Results file not found. No previous results to recover." << std::endl;
        return 0;
    }

    int enemy_y, uuv_x, uuv_y, uuv_z;
    double average_TL;
    char comma;
    int count = 0;

    std::string line;
    while (std::getline(file, line)) {
        // 跳过空行
        if (line.empty()) continue;

        std::istringstream iss(line);
        if (iss >> enemy_y >> comma >> uuv_x >> comma >> uuv_y >> comma >> uuv_z >> comma >> average_TL) {
            // 恢复 ans 数组
            ans[enemy_y][uuv_x][uuv_y][uuv_z] = average_TL;
            // 标记为已计算和已写入
            visited[enemy_y][uuv_x][uuv_y][uuv_z] = true;
            written[enemy_y][uuv_x][uuv_y][uuv_z] = true;
            count++;
        }
    }

    file.close();
    std::cout << "Loaded " << count << " previously computed results from file." << std::endl;
    return count;
}

int CheckpointManager::periodic_write()
{
    int new_results = 0;
    std::vector<std::tuple<int, int, int, int, double>> results_to_write;

    // 扫描所有坐标，找出新完成的结果（visited=true, written=false）
    for (int e_y = 0; e_y < 101; ++e_y) {
        for (int u_x = 0; u_x < 101; ++u_x) {
            for (int u_y = 0; u_y < 101; ++u_y) {
                for (int u_z = 0; u_z < 11; ++u_z) {
                    if (visited[e_y][u_x][u_y][u_z] && written[e_y][u_x][u_y][u_z] == false) {
                        results_to_write.emplace_back(e_y, u_x, u_y, u_z, ans[e_y][u_x][u_y][u_z]);
                    }
                }
            }
        }
    }

    // 如果没有新结果，直接返回
    if (results_to_write.empty()) {
        return 0;
    }

    // 使用 mutex 保护文件写入
    {
        std::lock_guard<std::mutex> lock(write_mutex_);

        std::ofstream file(results_file_, std::ios::app);  // 追加模式
        if (!file.is_open()) {
            std::cerr << "Failed to open results file for appending." << std::endl;
            return 0;
        }

        for (const auto& result : results_to_write) {
            int e_y, u_x, u_y, u_z;
            double tl;
            std::tie(e_y, u_x, u_y, u_z, tl) = result;

            // 使用与原代码相同的格式
            file << e_y << "," << u_x << "," << u_y << "," << u_z << "," 
                 << std::fixed << std::setprecision(4) << tl << "\n";

            // 标记为已写入
            written[e_y][u_x][u_y][u_z] = true;
            new_results++;
        }

        file.close();
    }

    return new_results;
}

bool CheckpointManager::append_result(int enemy_y, int uuv_x, int uuv_y, int uuv_z, double average_TL)
{
    std::lock_guard<std::mutex> lock(write_mutex_);

    std::ofstream file(results_file_, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Failed to open results file for appending." << std::endl;
        return false;
    }

    file << enemy_y << "," << uuv_x << "," << uuv_y << "," << uuv_z << "," 
         << std::fixed << std::setprecision(4) << average_TL << "\n";

    file.close();

    // 标记为已写入
    written[enemy_y][uuv_x][uuv_y][uuv_z] = true;

    return true;
}

int CheckpointManager::count_visited() const
{
    int count = 0;
    for (int e_y = 0; e_y < 101; ++e_y) {
        for (int u_x = 0; u_x < 101; ++u_x) {
            for (int u_y = 0; u_y < 101; ++u_y) {
                for (int u_z = 0; u_z < 11; ++u_z) {
                    if (visited[e_y][u_x][u_y][u_z]) {
                        count++;
                    }
                }
            }
        }
    }
    return count;
}

int CheckpointManager::count_written() const
{
    int count = 0;
    for (int e_y = 0; e_y < 101; ++e_y) {
        for (int u_x = 0; u_x < 101; ++u_x) {
            for (int u_y = 0; u_y < 101; ++u_y) {
                for (int u_z = 0; u_z < 11; ++u_z) {
                    if (written[e_y][u_x][u_y][u_z]) {
                        count++;
                    }
                }
            }
        }
    }
    return count;
}
