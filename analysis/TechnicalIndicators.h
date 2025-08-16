#pragma once
#include "../core/Types.h"
#include <vector>
#include <deque>
#include <cmath>

namespace hft {
namespace analysis {

// 技术指标计算器
class TechnicalIndicators {
public:
    // 移动平均线
    static std::vector<double> SMA(const std::vector<double>& prices, int period) {
        std::vector<double> sma(prices.size());
        double sum = 0;
        
        for (size_t i = 0; i < prices.size(); ++i) {
            sum += prices[i];
            if (i >= period) {
                sum -= prices[i - period];
                sma[i] = sum / period;
            } else {
                sma[i] = sum / (i + 1);
            }
        }
        return sma;
    }
    
    // 指数移动平均线
    static std::vector<double> EMA(const std::vector<double>& prices, int period) {
        std::vector<double> ema(prices.size());
        double multiplier = 2.0 / (period + 1);
        ema[0] = prices[0];
        
        for (size_t i = 1; i < prices.size(); ++i) {
            ema[i] = (prices[i] - ema[i-1]) * multiplier + ema[i-1];
        }
        return ema;
    }
    
    // 相对强弱指标(RSI)
    static std::vector<double> RSI(const std::vector<double>& prices, int period) {
        std::vector<double> rsi(prices.size());
        std::vector<double> gains, losses;
        
        for (size_t i = 1; i < prices.size(); ++i) {
            double diff = prices[i] - prices[i-1];
            gains.push_back(std::max(diff, 0.0));
            losses.push_back(std::max(-diff, 0.0));
        }
        
        std::vector<double> avgGain = SMA(gains, period);
        std::vector<double> avgLoss = SMA(losses, period);
        
        for (size_t i = 0; i < avgGain.size(); ++i) {
            if (avgLoss[i] == 0) {
                rsi[i+1] = 100;
            } else {
                double rs = avgGain[i] / avgLoss[i];
                rsi[i+1] = 100 - (100 / (1 + rs));
            }
        }
        return rsi;
    }
    
    // 布林带
    struct BollingerBands {
        std::vector<double> upper;
        std::vector<double> middle;
        std::vector<double> lower;
    };
    
    static BollingerBands BBANDS(const std::vector<double>& prices, 
                                int period, double stdDev = 2.0) {
        BollingerBands bb;
        bb.middle = SMA(prices, period);
        bb.upper.resize(prices.size());
        bb.lower.resize(prices.size());
        
        for (size_t i = period-1; i < prices.size(); ++i) {
            double sum = 0;
            for (int j = 0; j < period; ++j) {
                double diff = prices[i-j] - bb.middle[i];
                sum += diff * diff;
            }
            double std = std::sqrt(sum / period);
            bb.upper[i] = bb.middle[i] + stdDev * std;
            bb.lower[i] = bb.middle[i] - stdDev * std;
        }
        return bb;
    }
    
    // MACD
    struct MACD {
        std::vector<double> macd;
        std::vector<double> signal;
        std::vector<double> histogram;
    };
    
    static MACD calculateMACD(const std::vector<double>& prices, 
                             int fastPeriod = 12, 
                             int slowPeriod = 26, 
                             int signalPeriod = 9) {
        MACD result;
        std::vector<double> fastEMA = EMA(prices, fastPeriod);
        std::vector<double> slowEMA = EMA(prices, slowPeriod);
        
        result.macd.resize(prices.size());
        for (size_t i = 0; i < prices.size(); ++i) {
            result.macd[i] = fastEMA[i] - slowEMA[i];
        }
        
        result.signal = EMA(result.macd, signalPeriod);
        result.histogram.resize(prices.size());
        for (size_t i = 0; i < prices.size(); ++i) {
            result.histogram[i] = result.macd[i] - result.signal[i];
        }
        return result;
    }
    
    // 成交量加权平均价格(VWAP)
    static std::vector<double> VWAP(const std::vector<double>& prices,
                                  const std::vector<double>& volumes) {
        std::vector<double> vwap(prices.size());
        double cumVolume = 0;
        double cumPriceVolume = 0;
        
        for (size_t i = 0; i < prices.size(); ++i) {
            cumPriceVolume += prices[i] * volumes[i];
            cumVolume += volumes[i];
            vwap[i] = cumPriceVolume / cumVolume;
        }
        return vwap;
    }
    
    // 动量指标
    static std::vector<double> Momentum(const std::vector<double>& prices, int period) {
        std::vector<double> momentum(prices.size());
        for (size_t i = period; i < prices.size(); ++i) {
            momentum[i] = prices[i] - prices[i - period];
        }
        return momentum;
    }
    
    // 平均真实范围(ATR)
    static std::vector<double> ATR(const std::vector<double>& high,
                                 const std::vector<double>& low,
                                 const std::vector<double>& close,
                                 int period) {
        std::vector<double> tr(high.size());
        std::vector<double> atr(high.size());
        
        for (size_t i = 1; i < high.size(); ++i) {
            double hl = high[i] - low[i];
            double hc = std::abs(high[i] - close[i-1]);
            double lc = std::abs(low[i] - close[i-1]);
            tr[i] = std::max(std::max(hl, hc), lc);
        }
        
        atr = EMA(tr, period);
        return atr;
    }
};

} // namespace analysis
} // namespace hft
