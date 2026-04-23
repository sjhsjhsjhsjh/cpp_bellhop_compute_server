#include "utils.h"

#include <iostream>
#include <iomanip>

// 辅助函数：打印 bhcParams 中的关键参数以验证读取是否正确
void PrintParams(const bhc::bhcParams<true>& params)
{
        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << "  PARAMS DIAGNOSTIC OUTPUT" << std::endl;
        std::cout << std::string(70, '=') << std::endl;

        // ==================== 基础信息 ====================
        std::cout << "\n[BASIC INFO]" << std::endl;
        std::cout << "  Title: " << params.Title << std::endl;
        std::cout << "  Frequency (fT): " << std::fixed << std::setprecision(2) << params.fT << " Hz" << std::endl;

        // ==================== 源和接收器位置信息 ====================
        std::cout << "\n[SOURCE & RECEIVER POSITIONS]" << std::endl;
        if (params.Pos != nullptr) {
                std::cout << "  Source X count: " << params.Pos->NSx << std::endl;
                std::cout << "  Source Y count: " << params.Pos->NSy << std::endl;
                std::cout << "  Source Z count: " << params.Pos->NSz << std::endl;
                std::cout << "  Receiver Z count: " << params.Pos->NRz << std::endl;
                std::cout << "  Receiver R count: " << params.Pos->NRr << std::endl;
                std::cout << "  Receiver Theta count: " << params.Pos->Ntheta << std::endl;
                std::cout << "  RcvrZ per range: " << params.Pos->NRz_per_range << std::endl;

                // 打印源位置样本
                if (params.Pos->NSx > 0 && params.Pos->Sx != nullptr) {
                        std::cout << "\n  Source X (first " << std::min(5, params.Pos->NSx) << " values): ";
                        for (int i = 0; i < std::min(5, params.Pos->NSx); ++i) {
                                std::cout << std::fixed << std::setprecision(2) << params.Pos->Sx[i] << " ";
                        }
                        if (params.Pos->NSx > 5) std::cout << "... (" << params.Pos->NSx << " total)";
                        std::cout << std::endl;
                }

                if (params.Pos->NSy > 0 && params.Pos->Sy != nullptr) {
                        std::cout << "  Source Y (first " << std::min(5, params.Pos->NSy) << " values): ";
                        for (int i = 0; i < std::min(5, params.Pos->NSy); ++i) {
                                std::cout << std::fixed << std::setprecision(2) << params.Pos->Sy[i] << " ";
                        }
                        if (params.Pos->NSy > 5) std::cout << "... (" << params.Pos->NSy << " total)";
                        std::cout << std::endl;
                }

                if (params.Pos->NSz > 0 && params.Pos->Sz != nullptr) {
                        std::cout << "  Source Z (first " << std::min(5, params.Pos->NSz) << " values): ";
                        for (int i = 0; i < std::min(5, params.Pos->NSz); ++i) {
                                std::cout << std::fixed << std::setprecision(2) << params.Pos->Sz[i] << " ";
                        }
                        if (params.Pos->NSz > 5) std::cout << "... (" << params.Pos->NSz << " total)";
                        std::cout << std::endl;
                }

                // 打印接收器位置样本
                if (params.Pos->NRr > 0 && params.Pos->Rr != nullptr) {
                        std::cout << "  Receiver R (first " << std::min(5, params.Pos->NRr) << " values): ";
                        for (int i = 0; i < std::min(5, params.Pos->NRr); ++i) {
                                std::cout << std::fixed << std::setprecision(2) << params.Pos->Rr[i] << " ";
                        }
                        if (params.Pos->NRr > 5) std::cout << "... (" << params.Pos->NRr << " total)";
                        std::cout << std::endl;
                }

                if (params.Pos->NRz > 0 && params.Pos->Rz != nullptr) {
                        std::cout << "  Receiver Z (first " << std::min(5, params.Pos->NRz) << " values): ";
                        for (int i = 0; i < std::min(5, params.Pos->NRz); ++i) {
                                std::cout << std::fixed << std::setprecision(2) << params.Pos->Rz[i] << " ";
                        }
                        if (params.Pos->NRz > 5) std::cout << "... (" << params.Pos->NRz << " total)";
                        std::cout << std::endl;
                }

                if (params.Pos->Ntheta > 0 && params.Pos->theta != nullptr) {
                        std::cout << "  Receiver Theta (first " << std::min(5, params.Pos->Ntheta) << " values): ";
                        for (int i = 0; i < std::min(5, params.Pos->Ntheta); ++i) {
                                std::cout << std::fixed << std::setprecision(2) << params.Pos->theta[i] << " ";
                        }
                        if (params.Pos->Ntheta > 5) std::cout << "... (" << params.Pos->Ntheta << " total)";
                        std::cout << std::endl;
                }
        }
        else {
                std::cout << "  [WARNING] Pos pointer is null!" << std::endl;
        }

        // ==================== 射线角度信息 ====================
        std::cout << "\n[RAY ANGLES]" << std::endl;
        if (params.Angles != nullptr) {
                std::cout << "  Alpha (Elevation):" << std::endl;
                std::cout << "    Count: " << params.Angles->alpha.n << std::endl;
                std::cout << "    Spacing: " << std::fixed << std::setprecision(4) << params.Angles->alpha.d << std::endl;
                std::cout << "    In Degrees: " << (params.Angles->alpha.inDegrees ? "YES" : "NO") << std::endl;
                if (params.Angles->alpha.n > 0 && params.Angles->alpha.angles != nullptr) {
                        std::cout << "    Values (first " << std::min(5, params.Angles->alpha.n) << "): ";
                        for (int i = 0; i < std::min(5, params.Angles->alpha.n); ++i) {
                                std::cout << std::fixed << std::setprecision(4) << params.Angles->alpha.angles[i] << " ";
                        }
                        if (params.Angles->alpha.n > 5) std::cout << "... (" << params.Angles->alpha.n << " total)";
                        std::cout << std::endl;
                }

                std::cout << "  Beta (Azimuth):" << std::endl;
                std::cout << "    Count: " << params.Angles->beta.n << std::endl;
                std::cout << "    Spacing: " << std::fixed << std::setprecision(4) << params.Angles->beta.d << std::endl;
                std::cout << "    In Degrees: " << (params.Angles->beta.inDegrees ? "YES" : "NO") << std::endl;
                if (params.Angles->beta.n > 0 && params.Angles->beta.angles != nullptr) {
                        std::cout << "    Values (first " << std::min(5, params.Angles->beta.n) << "): ";
                        for (int i = 0; i < std::min(5, params.Angles->beta.n); ++i) {
                                std::cout << std::fixed << std::setprecision(4) << params.Angles->beta.angles[i] << " ";
                        }
                        if (params.Angles->beta.n > 5) std::cout << "... (" << params.Angles->beta.n << " total)";
                        std::cout << std::endl;
                }
        }
        else {
                std::cout << "  [WARNING] Angles pointer is null!" << std::endl;
        }

        // ==================== 频率信息 ====================
        std::cout << "\n[FREQUENCY INFO]" << std::endl;
        if (params.freqinfo != nullptr) {
                std::cout << "  Central Frequency: " << std::fixed << std::setprecision(2) << params.freqinfo->freq0 << " Hz" << std::endl;
                std::cout << "  Frequency Count: " << params.freqinfo->Nfreq << std::endl;
                if (params.freqinfo->Nfreq > 0 && params.freqinfo->freqVec != nullptr) {
                        std::cout << "  Frequencies (first " << std::min(5, params.freqinfo->Nfreq) << " values): ";
                        for (int i = 0; i < std::min(5, params.freqinfo->Nfreq); ++i) {
                                std::cout << std::fixed << std::setprecision(2) << params.freqinfo->freqVec[i] << " ";
                        }
                        if (params.freqinfo->Nfreq > 5) std::cout << "... (" << params.freqinfo->Nfreq << " total)";
                        std::cout << std::endl;
                }
        }
        else {
                std::cout << "  [WARNING] freqinfo pointer is null!" << std::endl;
        }

        // ==================== 波束配置 ====================
        std::cout << "\n[BEAM CONFIGURATION]" << std::endl;
        if (params.Beam != nullptr) {
                std::cout << "  Number of Beams: " << params.Beam->NBeams << std::endl;
                std::cout << "  Beam Type: " << params.Beam->Type << std::endl;
                std::cout << "  Run Type: " << params.Beam->RunType << std::endl;
                std::cout << "  Component: " << params.Beam->Component << std::endl;
                std::cout << "  Number of Images: " << params.Beam->Nimage << std::endl;
                std::cout << "  Delta S: " << std::fixed << std::setprecision(4) << params.Beam->deltas << std::endl;
                std::cout << "  Epsilon Multiplier: " << std::fixed << std::setprecision(4) << params.Beam->epsMultiplier << std::endl;
                std::cout << "  Range Loop: " << std::fixed << std::setprecision(2) << params.Beam->rLoop << std::endl;
        }
        else {
                std::cout << "  [WARNING] Beam pointer is null!" << std::endl;
        }

        // ==================== 声速剖面 (SSP) 信息 ====================
        std::cout << "\n[SOUND SPEED PROFILE (SSP)]" << std::endl;
        if (params.ssp != nullptr) {
                std::cout << "  Number of Points: " << params.ssp->NPts << std::endl;
                std::cout << "  SSP Type: " << params.ssp->Type << std::endl;
                std::cout << "  Range in Km: " << (params.ssp->rangeInKm ? "YES" : "NO") << std::endl;

                if (params.ssp->NPts > 0) {
                        std::cout << "  Depth values (first " << std::min(5, params.ssp->NPts) << "): ";
                        for (int i = 0; i < std::min(5, params.ssp->NPts); ++i) {
                                std::cout << std::fixed << std::setprecision(2) << params.ssp->z[i] << " ";
                        }
                        if (params.ssp->NPts > 5) std::cout << "... (" << params.ssp->NPts << " total)";
                        std::cout << std::endl;

                        std::cout << "  Sound Speed (first " << std::min(5, params.ssp->NPts) << "): ";
                        for (int i = 0; i < std::min(5, params.ssp->NPts); ++i) {
                                std::cout << std::fixed << std::setprecision(2) << params.ssp->c[i] << " ";
                        }
                        if (params.ssp->NPts > 5) std::cout << "... (" << params.ssp->NPts << " total)";
                        std::cout << std::endl;
                }
        }
        else {
                std::cout << "  [WARNING] ssp pointer is null!" << std::endl;
        }

        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << "  END OF PARAMS DIAGNOSTIC OUTPUT" << std::endl;
        std::cout << std::string(70, '=') << "\n" << std::endl;
}
