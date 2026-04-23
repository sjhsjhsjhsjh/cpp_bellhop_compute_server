#include <cmath>
#include <vector>
#include <limits>

#include "read_output.h"



// ============================================================================
// 函数1：计算单个接收器的传播损失
// ============================================================================

/**
 * 从复数压力计算传播损失（dB）
 *
 * @param pressure 复数声压
 * @param P_ref 参考压力（默认1.0 µPa）
 * @return 传播损失（dB），无信号时返回 TL_NO_SIGNAL
 */
float ComputeTL(const bhc::cpxf& pressure, float P_ref)
{
        // 计算幅度
        float magnitude = std::abs(pressure);

        // 处理幅度为零或非常小的情况
        if (magnitude < MIN_MAGNITUDE) {
                return TL_NO_SIGNAL;
        }

        // 计算比值
        float ratio = magnitude / P_ref;

        // 检查比值是否有效
        if (ratio <= 0.0f || !std::isfinite(ratio)) {
                return TL_NO_SIGNAL;
        }

        // 计算TL（dB）
        float TL_dB = -20.0f * std::log10(ratio);

        // 检查结果是否为有效数值
        if (!std::isfinite(TL_dB)) {
                return TL_NO_SIGNAL;
        }

        return TL_dB;
}

// ============================================================================
// 函数2：计算所有接收器的传播损失
// ============================================================================

/**
 * 遍历所有接收器并计算传播损失，结果保存到TL数组中
 *
 * @param params 仿真参数
 * @param outputs 仿真输出
 * @param tlField 输出的传播损失数组（会被初始化）
 * @param P_ref 参考压力（默认1.0 µPa）
 */
void compute_all_TL(
        const bhc::bhcParams<true>& params,           // 支持3D
        const bhc::bhcOutputs<true, true>& outputs,
        TLField& tlField,
        float P_ref)
{
        // 初始化TL数组
        tlField.Ntheta = params.Pos->Ntheta;
        tlField.NRz = params.Pos->NRz;
        tlField.NRr = params.Pos->NRr;
        tlField.data.clear();
        tlField.data.resize(tlField.Ntheta * tlField.NRz * tlField.NRr);

        // 遍历所有接收器
        for (int itheta = 0; itheta < tlField.Ntheta; ++itheta) {
                for (int iz = 0; iz < tlField.NRz; ++iz) {
                        for (int ir = 0; ir < tlField.NRr; ++ir) {
                                // 计算索引（源深度为0，源X、Y为0）
                                size_t idx = (((0 * params.Pos->NSy + 0)
                                        * params.Pos->Ntheta + itheta)
                                        * params.Pos->NRz + iz)
                                        * params.Pos->NRr + ir;

                                // 获取复数压力
                                bhc::cpxf p = outputs.uAllSources[idx];

                                // 计算TL
                                float TL_dB = ComputeTL(p, P_ref);

                                // 保存到TL数组
                                TLPoint& point = tlField(itheta, iz, ir);
                                point.bearing = params.Pos->theta[itheta];
                                point.depth = params.Pos->Rz[iz];
                                point.range = params.Pos->Rr[ir];
                                point.TL_dB = TL_dB;
                                point.valid = (TL_dB < TL_NO_SIGNAL - 1.0f);
                        }
                }
        }
}

double compute_average_TL(const TLField& tlField)
{
        double sum_TL = 0.0;
        int count = 0;

        for (const auto& point : tlField.data) {
                if (point.valid) {
                        sum_TL += point.TL_dB;
                        count++;
                }
        }

        if (count > 0) {
                double average_TL = sum_TL / count;
                return average_TL;
        } else {
                return 0.0;
        }
}