#include "read_npz.h"
#include <cnpy.h>
#include <stdexcept>



PointPair::PointPair(const Position3D& src, const Position3D& rcv) : source(src), receiver(rcv)
{
    
}

PointPair::PointPair(int enemy_y, int uuv_x, int uuv_y, int uuv_z)
{
    source = { static_cast<double>(uuv_x), static_cast<double>(uuv_y), static_cast<double>(uuv_z) };
    receiver = { enemy_x, static_cast<double>(enemy_y), enemy_z };
}

void PointPair::generate_key()
{
    key_info = generate_cache_key(source, receiver);
}

void PointPair::compute_self_bellhop_parameters()
{
    bellhop_params = compute_bellhop_parameters(source, receiver);
}

std::vector<PointPair> loadKeyPairsFromNPZ(const std::string& filepath)
{
    std::vector<PointPair> result;
    
    try {
        // 加载 NPZ 文件
        cnpy::npz_t npz_data = cnpy::npz_load(filepath);
        
        // 获取 key_pairs 数组
        cnpy::NpyArray& key_pairs_array = npz_data["key_pairs"];
        
        // 验证数据形状
        if (key_pairs_array.shape.size() != 2 || key_pairs_array.shape[1] != 4) {
                throw std::runtime_error("Invalid key_pairs shape: expected (N, 4)");
        }
        
        size_t num_pairs = key_pairs_array.shape[0];
        
        // 获取数据指针（假设数据类型为 int）
        int* data = key_pairs_array.data<int>();
        
        // 预留空间以提高效率
        result.reserve(num_pairs);
        
        // 逐行读取数据：[enemy_y, uuv_x, uuv_y, uuv_z]
        for (size_t i = 0; i < num_pairs; ++i) {
            int enemy_y = data[i * 4 + 0];
            int uuv_x   = data[i * 4 + 1];
            int uuv_y   = data[i * 4 + 2];
            int uuv_z   = data[i * 4 + 3];
            
            result.emplace_back(enemy_y, uuv_x, uuv_y, uuv_z);

            if (i == 0) {
                std::cout << "First key pair loaded: enemy_y=" << enemy_y
                            << ", uuv_x=" << uuv_x
                            << ", uuv_y=" << uuv_y
                            << ", uuv_z=" << uuv_z << std::endl;
            }
        }
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to load key_pairs from NPZ: ") + e.what());
    }
    
    return result;
}
