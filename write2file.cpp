#include "write2file.h"

#include <fstream>
#include <iomanip>

void write_tl2file(PointPair& pair, const TLField& tlField)
{
        // 将结果输出到文件
        std::string output_filename = "tl_" + pair.key_info.cache_key + ".txt";
        std::ofstream output_file(output_filename);
        output_file << std::fixed << std::setprecision(4);
        output_file << pair.source.x << " " << pair.source.y << " " << pair.source.z << " "
                << pair.receiver.x << " " << pair.receiver.y << " " << pair.receiver.z << std::endl;
        for (const auto& point : tlField.data) {
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