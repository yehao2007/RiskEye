#include "MarketEnvironmentAnalyzer.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace hft {
namespace strategy {

MarketEnvironmentAnalyzer::MarketEnvironmentAnalyzer() : m_volatilityThreshold(0.02), m_trendThreshold(0.01) {
}

MarketEnvironmentAnalyzer::~MarketEnvironmentAnalyzer() {
}

void MarketEnvironmentAnalyzer::setParameters(double volatilityThreshold, double trendThreshold) {
    m_volatilityThreshold = volatilityThreshold;
    m_trendThreshold = trendThreshold;
}

MarketState MarketEnvironmentAnalyzer::analyzeMarketState(const std::vector<double>& priceData) {
    if (priceData.size() < 20) {
        std::cerr << "Not enough price data for analysis" << std::endl;
        return MarketState::RANGING;
    }

    // 计算收益率
    std::vector<double> returns;
    for (size_t i = 1; i < priceData.size(); ++i) {
        returns.push_back((priceData[i] - priceData[i-1]) / priceData[i-1]);
    }

    // 计算波动率
    double meanReturn = 0.0;
    for (double ret : returns) {
        meanReturn += ret;
    }
    meanReturn /= returns.size();

    double volatility = 0.0;
    for (double ret : returns) {
        volatility += std::pow(ret - meanReturn, 2);
    }
    volatility = std::sqrt(volatility / returns.size());

    // 计算趋势
    // 使用线性回归计算趋势
    std::vector<double> x(returns.size());
    for (size_t i = 0; i < x.size(); ++i) {
        x[i] = static_cast<double>(i);
    }

    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    for (size_t i = 0; i < returns.size(); ++i) {
        sumX += x[i];
        sumY += returns[i];
        sumXY += x[i] * returns[i];
        sumX2 += x[i] * x[i];
    }

    double n = static_cast<double>(returns.size());
    double slope = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);

    // 分类市场状态
    if (volatility > m_volatilityThreshold * 2) {
        if (slope > m_trendThreshold) {
            return MarketState::RALLY;
        } else if (slope < -m_trendThreshold) {
            return MarketState::CRASH;
        } else {
            return MarketState::HIGH_VOLATILITY;
        }
    } else if (volatility > m_volatilityThreshold) {
        if (std::abs(slope) > m_trendThreshold) {
            return MarketState::TRENDING;
        } else {
            return MarketState::HIGH_VOLATILITY;
        }
    } else {
        if (std::abs(slope) > m_trendThreshold) {
            return MarketState::TRENDING;
        } else {
            return MarketState::LOW_VOLATILITY;
        }
    }
}

void MarketEnvironmentAnalyzer::onMarketData(const network::MarketData& data) {
    // 存储最新价格
    m_priceHistory.push_back(data.lastPrice);

    // 保持历史数据在一定长度内
    if (m_priceHistory.size() > 100) {
        m_priceHistory.erase(m_priceHistory.begin());
    }

    // 分析市场状态
    if (m_priceHistory.size() >= 20) {
        m_currentMarketState = analyzeMarketState(m_priceHistory);

        // 通知所有观察者
        notifyObservers(m_currentMarketState);
    }
}

void MarketEnvironmentAnalyzer::registerObserver(MarketStateObserver* observer) {
    m_observers.push_back(observer);
}

void MarketEnvironmentAnalyzer::unregisterObserver(MarketStateObserver* observer) {
    auto it = std::find(m_observers.begin(), m_observers.end(), observer);
    if (it != m_observers.end()) {
        m_observers.erase(it);
    }
}

void MarketEnvironmentAnalyzer::notifyObservers(MarketState state) {
    for (MarketStateObserver* observer : m_observers) {
        observer->onMarketStateChanged(state);
    }
}

} // namespace strategy
} // namespace hft