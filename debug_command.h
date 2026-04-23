#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstdio>

#if defined(_MSC_VER)
    #pragma warning( disable : 4828 )
#endif

// 获取当前时间的字符串
#define CURRENT_TIME() []() -> std::string { \
    auto now = std::chrono::system_clock::now(); \
    auto in_time_t = std::chrono::system_clock::to_time_t(now); \
    std::stringstream ss; \
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X"); \
    return ss.str(); \
}()


// 自带换行的颜色输出
#define GREEN(STRING) std::cout<<"\033[32m"<<STRING<<"\033[m\n"
#define RED(STRING) std::cout<<"\033[31m"<<STRING<<"\033[m\n"
#define YELLOW(STRING) std::cout<<"\033[33m"<<STRING<<"\033[m\n"

// 不带换行符的颜色输出
#define YELLOW_WITHOUTENTER(STRING) std::cout<<"\033[33m"<<STRING<<"\033[m"
#define GREEN_WITHOUTENTER(STRING) std::cout<<"\033[32m"<<STRING<<"\033[m"
#define RED_WITHOUTENTER(STRING) std::cout<<"\033[31m"<<STRING<<"\033[m"

// 单纯的颜色控制
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_NORMAL "\033[m"

// 格式输出
#define SUCCESS(STRING) std::cout << COLOR_GREEN << "[SUCC]: " << STRING << COLOR_NORMAL << std::endl;
#define INFO(STRING) std::cout << "[INFO]: " << STRING << std::endl;
#define WARN(STRING) std::cout << COLOR_YELLOW << "[WARN]: " << STRING << COLOR_NORMAL << std::endl;
#define ERROR(STRING) std::cout << COLOR_RED << "[ERROR]: " << STRING << COLOR_NORMAL << std::endl;

#define INFO_format(fmt, ...)    do { printf("[INFO]: "  fmt "\n", ##__VA_ARGS__); } while (0)
#define WARN_format(fmt, ...)    do { printf(COLOR_YELLOW "[WARN]: "  fmt "\n" COLOR_NORMAL, ##__VA_ARGS__); } while (0)
#define ERROR_format(fmt, ...)   do { printf(COLOR_RED    "[ERROR]: " fmt "\n" COLOR_NORMAL, ##__VA_ARGS__); } while (0)
#define SUCCESS_format(fmt, ...) do { printf(COLOR_GREEN  "[SUCC]: "  fmt "\n" COLOR_NORMAL, ##__VA_ARGS__); } while (0)

// 数值输出
#define DEBUG_VAR1(VAR) std::cout << #VAR << " = " << VAR << std::endl;
#define DEBUG_VAR2(VAR1, VAR2) std::cout << #VAR1 << " = " << VAR1 << "    " << #VAR2 << " = " << VAR2 << std::endl;

// 函数名与行数
#define ERROR_LOCATION(STRING) std::cout << COLOR_RED << "[ERROR]: \"" << __FILE__ << "\": " << __FUNCTION__ << " Line " << __LINE__ << ": " << STRING << COLOR_NORMAL << std::endl;
#define WARN_LOCATION(STRING) std::cout << COLOR_YELLOW << "[WARN]: \"" << __FILE__ << "\": " << __FUNCTION__ << " Line " << __LINE__ << ": " << STRING << COLOR_NORMAL << std::endl;
#define SUCC_LOCATION(STRING) std::cout << COLOR_GREEN << "[SUCC]: \"" << __FILE__ << "\": " << __FUNCTION__ << " Line " << __LINE__ << ": " << STRING << COLOR_NORMAL << std::endl;
#define INFO_LOCATION(STRING) std::cout << "[INFO]: \"" << __FILE__ << "\": " << __FUNCTION__ << " Line " << __LINE__ << ": " << STRING << std::endl;

#define INFO_TIME(STRING) std::cout << CURRENT_TIME() << "    " << STRING << std::endl;

// 检查指针是否为空指针
#define IS_NULLPTR_RETURN_VOID(PTR) \
    if (PTR == nullptr) { \
        ERROR_LOCATION("Pointer " #PTR " is nullptr!"); \
        return; \
    }