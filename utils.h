#include "bhc/bhc.hpp"

void PrintParams(const bhc::bhcParams<true>& params);

// Helper to setup vectors from ranges. No checking.
// 从 Bellhop C++ 源代码里拷出来的，用于填充数组
template<class T>
void SetupVector(T* arr, T low, T high, int size)
{
        for (int i = 0; i < size; ++i) {
                arr[i] = low + double(i) / double(size - 1) * (high - low);
        }
}