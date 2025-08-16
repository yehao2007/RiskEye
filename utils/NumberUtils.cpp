#include "NumberUtils.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <numeric>

namespace hft {
namespace utils {

double NumberUtils::roundTo(double value, int decimal_places) {
    double factor = std::pow(10.0, decimal_places);
    return std::round(value * factor) / factor;
}

double NumberUtils::linearInterpolate(double x0, double y0, double x1, double y1, double x) {
    if (x0 == x1) {
        return y0;
    }
    return y0 + (y1 - y0) * (x - x0) / (x1 - x0);
}

double NumberUtils::mean(const std::vector<double>& values) {
    if (values.empty()) {
        return 0.0;
    }
    double sum = std::accumulate(values.begin(), values.end(), 0.0);
    return sum / values.size();
}

double NumberUtils::standardDeviation(const std::vector<double>& values) {
    if (values.size() < 2) {
        return 0.0;
    }
    double avg = mean(values);
    double variance = 0.0;
    for (double value : values) {
        variance += (value - avg) * (value - avg);
    }
    variance /= values.size() - 1;
    return std::sqrt(variance);
}

std::string NumberUtils::formatWithThousandsSeparator(uint64_t number) {
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::setprecision(0) << number;
    return ss.str();
}

bool NumberUtils::stringToNumber(const std::string& str, double& result) {
    try {
        result = std::stod(str);
        return true;
    } catch (...) {
        return false;
    }
}

bool NumberUtils::stringToNumber(const std::string& str, int64_t& result) {
    try {
        result = std::stoll(str);
        return true;
    } catch (...) {
        return false;
    }
}

bool NumberUtils::stringToNumber(const std::string& str, uint64_t& result) {
    try {
        result = std::stoull(str);
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace utils
} // namespace hft