#include "bellhop_params.h"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <functional>
#include <stdexcept>

// M_PI 不是标准常数，在某些系统中需要定义
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 验证浮点数有效性
Position3D normalize_position(double x, double y, double z) {
    if (std::isnan(x) || std::isnan(y) || std::isnan(z) ||
        std::isinf(x) || std::isinf(y) || std::isinf(z)) {
        throw std::invalid_argument(
            "Position coordinates contain NaN or Inf values");
    }
    return Position3D{x, y, z};
}

// ============================================================================
// 实现：relative_polar
// ============================================================================
std::pair<double, double> relative_polar(
    double ux, double uy, double ex, double ey) {
    
    double dx = ex - ux;
    double dy = ey - uy;
    
    // 计算距离
    double r = std::hypot(dx, dy);
    
    // 计算方向角（弧度）
    double theta_rad = std::atan2(dy, dx);
    
    // 转换为度数 [0, 360)
    double theta_deg = theta_rad * 180.0 / M_PI;
    if (theta_deg < 0.0) {
        theta_deg += 360.0;
    }
    
    return std::make_pair(r, theta_deg);
}

// ============================================================================
// 实现：angle_window
// ============================================================================
std::pair<double, double> angle_window(
    double center_deg, double half_width_deg) {
    
    double start = center_deg - half_width_deg;
    double end = center_deg + half_width_deg;
    
    // 边界约束
    if (start < 0.0) {
        start = 0.0;
    }
    if (end > 360.0) {
        end = 360.0;
    }
    
    // 失效时修正
    if (end <= start) {
        end = std::min(360.0, start + std::max(0.5, 2.0 * half_width_deg));
    }
    
    return std::make_pair(start, end);
}

// ============================================================================
// 实现：range_window
// ============================================================================
std::pair<double, double> range_window(double distance_m) {
    double r_min = std::max(50.0, distance_m - 500.0);
    double r_max = std::max(r_min + 50, distance_m + 500.0);
    
    return std::make_pair(r_min, r_max);
}

// ============================================================================
// 实现：compute_bellhop_parameters
// ============================================================================
BellhopEnvParameters compute_bellhop_parameters(
    const Position3D& uuv_pos, const Position3D& enemy_pos) {
    
    // 标准化输入（验证有效性）
    Position3D uuv = normalize_position(uuv_pos.x, uuv_pos.y, uuv_pos.z);
    Position3D enemy = normalize_position(
        enemy_pos.x, enemy_pos.y, enemy_pos.z);
    
    // 计算相对极坐标
    auto [distance_m, bearing_deg] = 
        relative_polar(uuv.x, uuv.y, enemy.x, enemy.y);
    
    // 计算范围窗口
    auto [r_min, r_max] = range_window(distance_m);
    
    // 计算方向窗口1（bearing）
    auto [bearing_start, bearing_end] = angle_window(bearing_deg, 2.0);
    
    // 计算方向窗口2（beta）
    auto [beta_start, beta_end] = angle_window(bearing_deg, 3.0);
    
    // 组装结果结构体
    BellhopEnvParameters params;
    params.source_x = uuv.x;
    params.source_y = uuv.y;
    params.source_z = uuv.z;
    params.receiver_x = enemy.x;
    params.receiver_y = enemy.y;
    params.receiver_z = enemy.z;
    params.distance_m = distance_m;
    params.bearing_deg = bearing_deg;
    params.range_min = r_min;
    params.range_max = r_max;
    params.bearing_start = bearing_start;
    params.bearing_end = bearing_end;
    params.beta_start = beta_start;
    params.beta_end = beta_end;
    
    return params;
}

// ============================================================================
// 实现：generate_cache_key
// ============================================================================
CacheKeyInfo generate_cache_key(
    const Position3D& uuv_pos, const Position3D& enemy_pos) {
    
    // 验证输入有效性
    Position3D uuv = normalize_position(uuv_pos.x, uuv_pos.y, uuv_pos.z);
    Position3D enemy = normalize_position(
        enemy_pos.x, enemy_pos.y, enemy_pos.z);
    
    // 构建字符串：将坐标值（精度 6 位小数）拼接
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    oss << uuv.x << "_" << uuv.y << "_" << uuv.z << "_"
        << enemy.x << "_" << enemy.y << "_" << enemy.z;
    std::string payload_str = oss.str();
    
    // 计算 hash
    std::hash<std::string> hasher;
    std::size_t hash_value = hasher(payload_str);
    
    // 将 hash 值转换为 32 字符十六进制字符串
    std::ostringstream hash_stream;
    hash_stream << std::hex << std::setfill('0') << std::setw(16)
                << hash_value;
    // 对于 64 位系统，std::size_t 通常是 64 位，但 std::setw(16) 只填充 16 个字符
    // 为了保证 32 字符，我们需要处理完整的 64 位值
    
    // 更好的方法：使用完整 128 位或 64 位的哈希
    std::string hash_str = hash_stream.str();
    
    // 确保有 32 个字符（可能不足，需要补零）
    // 对于 std::size_t (通常 64 位)，std::setw(16) 给出最多 16 字符十六进制
    // 我们需要另一个 hash 来扩展到 32 字符
    std::hash<std::string> hasher2;
    std::size_t hash_value2 = hasher2(payload_str + "_");
    
    std::ostringstream hash_stream2;
    hash_stream2 << std::hex << std::setfill('0') << std::setw(16)
                 << hash_value2;
    std::string hash_str2 = hash_stream2.str();
    
    // 合并两个哈希以得到 32 字符
    std::string cache_key = hash_str + hash_str2;
    // 若超过 32，取前 32；若不足，使用 std::setw 的填充
    if (cache_key.length() > 32) {
        cache_key = cache_key.substr(0, 32);
    } else if (cache_key.length() < 32) {
        // 补零到 32 字符
        cache_key = std::string(32 - cache_key.length(), '0') + cache_key;
    }
    
    // 生成 filename stem（使用完整 32 字符 cache_key 避免碰撞风险）
    std::string filename_stem = "env_" + cache_key;
    
    CacheKeyInfo key_info;
    key_info.cache_key = cache_key;
    key_info.filename_stem = filename_stem;
    
    return key_info;
}