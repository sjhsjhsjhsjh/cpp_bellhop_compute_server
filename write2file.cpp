#include "write2file.h"

#include <fstream>
#include <iomanip>
#include <algorithm>

void write_tl2file(PointPair& pair, const TLField& tlField)
{
        // 将结果输出到文件
        std::string output_filename = "tl_" + pair.key_info.cache_key + ".txt";
        std::ofstream output_file(output_filename);
        output_file << std::fixed << std::setprecision(4);
        output_file << pair.source.x << " " << pair.source.y << " " << pair.source.z << " "
                << pair.receiver.x << " " << pair.receiver.y << " " << pair.receiver.z << std::endl;

        // 将 TLField 中的 data 深拷贝一份，然后进行排序（按 depth、range、bearing 顺序）
        auto sorted_data = tlField.data;
        std::sort(sorted_data.begin(), sorted_data.end(), [](const TLPoint& a, const TLPoint& b) {
                if (a.depth != b.depth) return a.depth < b.depth;
                if (a.range != b.range) return a.range < b.range;
                return a.bearing < b.bearing;
        });

        for (const auto& point : sorted_data) {
                if (point.valid) {
                        output_file << std::fixed << std::setprecision(4)
                                << point.depth << " "
                                << point.range << " "
                                << point.bearing << " "
                                << point.TL_dB << std::endl;
                }
        }
        output_file.close();
}

void write_param2env(const PointPair& pair, bhc::bhcParams<true>& params)
{
        std::string env_filename = "env_" + pair.key_info.cache_key;
        bhc::writeenv(params, env_filename.c_str());
}