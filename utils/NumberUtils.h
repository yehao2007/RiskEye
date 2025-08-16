#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace hft {
namespace utils {

// 数值工具类
class NumberUtils {
public:
    // 四舍五入到指定位数
    static double roundTo(double value, int decimal_places);

    // 检查数值是否在范围内
    template<typename T>
    static bool isInRange(T value, T min, T max);

    // 线性插值
    static double linearInterpolate(double x0, double y0, double x1, double y1, double x);

    // 计算向量的平均值
    static double mean(const std::vector<double>& values);

    // 计算向量的标准差
    static double standardDeviation(const std::vector<double>& values);

    // 将数字转换为字符串，添加千位分隔符
    static std::string formatWithThousandsSeparator(uint64_t number);

    // 将字符串转换为数字
    static bool stringToNumber(const std::string& str, double& result);
    static bool stringToNumber(const std::string& str, int64_t& result);
    static bool stringToNumber(const std::string& str, uint64_t& result);
};

// 模板实现
 template<typename T>
 bool NumberUtils::isInRange(T value, T min, T max) {
    return value >= min && value <= max;
}

} // namespace utils
} // namespace hft