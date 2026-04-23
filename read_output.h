// 定义传播损失的特殊值
constexpr float TL_NO_SIGNAL = 120.0f;      // 无信号时的标记值
constexpr float MIN_MAGNITUDE = 1e-10f;     // 最小有效幅度阈值

#include "bhc/bhc.hpp"

// ============================================================================
// 数据结构：传播损失信息
// ============================================================================

struct TLPoint {
        float bearing;      // 方位角（度）
        float depth;        // 接收器深度（m）
        float range;        // 接收器距离（m）
        float TL_dB;        // 传播损失（dB）
        bool valid;         // 是否为有效数据
};

struct TLField {
        std::vector<TLPoint> data;
        int Ntheta;         // 方位角数量
        int NRz;            // 深度数量
        int NRr;            // 距离数量

        // 通过索引访问TL值
        TLPoint& operator()(int itheta, int iz, int ir)
        {
                return data[((itheta * NRz + iz) * NRr + ir)];
        }

        const TLPoint& operator()(int itheta, int iz, int ir) const
        {
                return data[((itheta * NRz + iz) * NRr + ir)];
        }
};

// ============================================================================
// 常数定义
// ============================================================================



float ComputeTL(const bhc::cpxf& pressure, float P_ref = 1.0f);

void compute_all_TL(
        const bhc::bhcParams<true>& params,
        const bhc::bhcOutputs<true, true>& outputs,
        TLField& tlField,
        float P_ref = 1.0f);

double compute_average_TL(const TLField& tlField);