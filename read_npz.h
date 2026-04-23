#define NPZ_FILE_PATH "key_points.npz"
#define enemy_x 2000
#define enemy_z 150

#include <iostream>
#include <vector>
#include <string>
#include "bellhop_params.h"

#ifndef READ_NPZ_H
#define READ_NPZ_H

class PointPair {
public:
        Position3D source;
        Position3D receiver;
        CacheKeyInfo key_info;
        BellhopEnvParameters bellhop_params;

public:
        PointPair(const Position3D& src, const Position3D& rcv);
        PointPair(int enemy_y, int uuv_x, int uuv_y, int uuv_z);

        void generate_key();

        void compute_self_bellhop_parameters();
};

/**
 * @brief 从 NPZ 文件读取所有 key_pairs 数据并转换为 PointPair 向量
 * @param filepath NPZ 文件路径，默认为 NPZ_FILE_PATH
 * @return 包含所有 PointPair 对象的向量
 * @throws std::runtime_error 如果文件读取失败或数据格式错误
 */
std::vector<PointPair> loadKeyPairsFromNPZ(const std::string& filepath = NPZ_FILE_PATH);

#endif // READ_NPZ_H