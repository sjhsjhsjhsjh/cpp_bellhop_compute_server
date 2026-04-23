#ifndef BELLHOP_PARAMS_H
#define BELLHOP_PARAMS_H

#include <string>
#include <optional>
#include <cmath>

/**
 * @file bellhop_params.h
 * @brief 无文件交互的 Bellhop 参数生成库（C++17）
 *
 * 从 Python generate_bellhop_env.py 中提取的纯计算逻辑。
 * 输入两个三维坐标点，输出所有 Bellhop env 文件所需的参数。
 * 无文件 I/O，所有计算在内存中完成。
 *
 * 坐标单位约定：
 * - x/y: km（公里）
 * - z: m（米）
 */

/**
 * @struct Position3D
 * @brief 三维坐标点
 */
struct Position3D {
    double x;  ///< X 坐标，单位 km
    double y;  ///< Y 坐标，单位 km
    double z;  ///< Z 坐标（深度），单位 m
};

/**
 * @struct BellhopEnvParameters
 * @brief Bellhop env 文件所需的所有计算参数
 *
 * 该结构体包含了原本要替换到 env 文件中的所有数据段。
 * 可直接用于文件生成或网络传输。
 */
struct BellhopEnvParameters {
    // === 源点参数（UUV/声源） ===
    double source_x;        ///< 声源 X 坐标，单位 m
    double source_y;        ///< 声源 Y 坐标，单位 m
    double source_z;        ///< 声源深度，单位 m
    
    // === 接收点参数（敌舰） ===
    double receiver_x;      ///< 接收点 X 坐标，单位 m
    double receiver_y;      ///< 接收点 Y 坐标，单位 m
    double receiver_z;      ///< 接收点深度，单位 m
    
    // === 相对极坐标 ===
    double distance_m;     ///< 源点到接收点的距离，单位 km
    double bearing_deg;     ///< 方向角（从 X 轴正向），单位°，范围 [0, 360)
    
    // === 范围窗口（Range window） ===
    double range_min;       ///< 最小距离，单位 m
    double range_max;       ///< 最大距离，单位 m
    
    // === 方向窗口1（Bearing window） ===
    double bearing_start;   ///< 方向窗口起始角，单位°
    double bearing_end;     ///< 方向窗口结束角，单位°
    
    // === 方向窗口2（Beta window） ===
    double beta_start;      ///< Beta 窗口起始角，单位°
    double beta_end;        ///< Beta 窗口结束角，单位°
};

/**
 * @struct CacheKeyInfo
 * @brief 缓存键和文件名信息
 */
struct CacheKeyInfo {
    std::string cache_key;      ///< 32字符十六进制缓存键
    std::string filename_stem;  ///< "env_{cache_key}"，用于生成文件名
};

// ============================================================================
// API 函数声明
// ============================================================================

/**
 * @brief 标准化位置坐标
 *
 * @param x X 坐标（km）
 * @param y Y 坐标（km）
 * @param z Z 坐标（m）
 *
 * @return Position3D 结构体，若输入有效
 * @throws std::invalid_argument 若坐标包含 NaN 或 Inf
 *
 * @example
 * auto pos = normalize_position(17.0, 10.0, 120.0);
 */
Position3D normalize_position(double x, double y, double z);

/**
 * @brief 计算源点到接收点的相对极坐标
 *
 * 将 (ux, uy) 和 (ex, ey) 转换为相对极坐标 (r_km, theta_deg)。
 * theta 以 X 轴正向为 0°，逆时针为正。
 *
 * @param ux 源点 X 坐标（km）
 * @param uy 源点 Y 坐标（km）
 * @param ex 接收点 X 坐标（km）
 * @param ey 接收点 Y 坐标（km）
 *
 * @return std::pair<double, double> 为 (distance_km, bearing_deg)，
 *         其中 bearing_deg 范围为 [0, 360)
 *
 * @example
 * auto [r, theta] = relative_polar(0.0, 0.0, 3.0, 4.0);
 * // r ≈ 5.0, theta ≈ 53.13
 */
std::pair<double, double> relative_polar(
    double ux, double uy, double ex, double ey);

/**
 * @brief 计算角度窗口 [start, end]
 *
 * 基于中心角和半宽度，生成一个被约束到 [0, 360] 范围内的角度窗口。
 * 若计算后 end <= start，则自动调整 end 为有效范围。
 *
 * @param center_deg 中心角度（°）
 * @param half_width_deg 窗口半宽度（°）
 *
 * @return std::pair<double, double> 为 (start_deg, end_deg)
 *
 * @note 特殊情况处理：
 * - 若 center - half_width < 0，则 start 被设为 0
 * - 若 center + half_width > 360，则 end 被设为 360
 * - 若调整后 end <= start，则 end = min(360, start + max(0.5, 2*half_width))
 *
 * @example
 * auto [start, end] = angle_window(90.0, 2.0);
 * // start = 88.0, end = 92.0
 */
std::pair<double, double> angle_window(
    double center_deg, double half_width_deg);

/**
 * @brief 计算范围窗口 [r_min, r_max]
 *
 * 基于中心距离，生成一个距离窗口。
 *
 * @param distance_km 中心距离（km）
 *
 * @return std::pair<double, double> 为 (r_min, r_max)
 *
 * @note 计算规则：
 * - r_min = max(0.05, distance_km - 0.5)
 * - r_max = max(r_min + 0.05, distance_km + 0.5)
 *
 * @example
 * auto [r_min, r_max] = range_window(10.0);
 * // r_min ≈ 9.5, r_max ≈ 10.5
 */
std::pair<double, double> range_window(double distance_km);

/**
 * @brief 计算完整的 Bellhop env 参数
 *
 * 主函数：根据源点和接收点坐标，计算所有必需的 Bellhop 参数。
 * 包括极坐标转换、范围/角度窗口计算。
 *
 * @param uuv_pos UUV（声源）位置
 * @param enemy_pos 敌舰（接收点）位置
 *
 * @return BellhopEnvParameters 结构体，包含所有计算结果
 * @throws std::invalid_argument 若位置坐标包含 NaN 或 Inf
 *
 * @example
 * Position3D uuv{17.0, 10.0, 120.0};
 * Position3D enemy{12.0, 8.0, 90.0};
 * auto params = compute_bellhop_parameters(uuv, enemy);
 * std::cout << "Distance: " << params.distance_km << " km\n";
 */
BellhopEnvParameters compute_bellhop_parameters(
    const Position3D& uuv_pos, const Position3D& enemy_pos);

/**
 * @brief 生成缓存键
 *
 * 基于两个坐标点的坐标值（精度 6 位小数）生成一个稳定的哈希键。
 * 该键可用作 Bellhop env/shd 文件的文件名前缀。
 *
 * 算法：
 * 1. 将两个坐标点的所有值拼接成字符串（格式：x_y_z_x_y_z）
 * 2. 对字符串应用 std::hash<std::string>
 * 3. 将哈希值转换为 32 字符十六进制字符串
 * 4. filename_stem = "env_{cache_key}"
 *
 * @param uuv_pos UUV 位置
 * @param enemy_pos 敌舰位置
 *
 * @return CacheKeyInfo 包含 32 字符十六进制 cache_key 和 filename_stem
 *
 * @note 特性：
 * - 相同输入保证生成相同缓存键
 * - 精度：坐标值取小数点后 6 位
 * - 无 template 文件哈希依赖（与 Python 版本不同）
 *
 * @example
 * Position3D uuv{17.0, 10.0, 120.0};
 * Position3D enemy{12.0, 8.0, 90.0};
 * auto key_info = generate_cache_key(uuv, enemy);
 * std::string filename = key_info.filename_stem + ".env";
 */
CacheKeyInfo generate_cache_key(
    const Position3D& uuv_pos, const Position3D& enemy_pos);


#endif // BELLHOP_PARAMS_H
