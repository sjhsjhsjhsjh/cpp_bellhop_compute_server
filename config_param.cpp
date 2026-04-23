#include "config_param.h"
#include "bellhop_params.h"
#include "utils.h"

void config_bhc_params(bhc::bhcParams<true>& params, const PointPair& pair)
{
        // 替换本次参数到 params 中，与 python 程序对照，核准无误
        // ✅️ 替换 声源x坐标
        params.Pos->Sx[0] = pair.bellhop_params.source_x;
        // ✅️ 替换 声源y坐标
        params.Pos->Sy[0] = pair.bellhop_params.source_y;
        // ✅️ 替换 声源z坐标
        params.Pos->Sz[0] = pair.bellhop_params.source_z;
        // ✅️ 替换 接收器深度 
        params.Pos->Rz[0] = pair.bellhop_params.receiver_z;
        // ✅️ 替换 接收器范围（r坐标） 对应 env 文件中的 bearing angles (degrees)
        SetupVector(params.Pos->Rr, pair.bellhop_params.range_min, pair.bellhop_params.range_max, 10);
        // ✅️ 替换 接收器方位（theta坐标）
        SetupVector(params.Pos->theta, pair.bellhop_params.bearing_start, pair.bellhop_params.bearing_end, 45);
        // ✅️ 替换 发射射线的俯仰角范围
        params.Angles->beta.inDegrees = true;
        SetupVector(params.Angles->beta.angles, pair.bellhop_params.beta_start, pair.bellhop_params.beta_end, 60);

}