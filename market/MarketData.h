#pragma once
#include "Types.h"
#include <string>
#include <map>
#include <queue>
#include <mutex>

namespace hft {
namespace market {

// Level 2行情数据
struct L2Data {
    std::string symbol;
    types::Timestamp timestamp;
    struct Level {
        types::Price price;
        types::Volume volume;
    };
    std::vector<Level> bids;
    std::vector<Level> asks;
};

// 成交数据
struct Trade {
    std::string symbol;
    types::Timestamp timestamp;
    types::Price price;
    types::Volume volume;
    types::Side side;
};

// 订单簿
class OrderBook {
public:
    void update(const L2Data& data);
    types::Price getBestBid() const;
    types::Price getBestAsk() const;
    types::Volume getVolumeAtPrice(types::Price price) const;
    double getMidPrice() const;
    double getVWAP() const;
    
private:
    std::string symbol_;
    std::map<types::Price, types::Volume> bids_;
    std::map<types::Price, types::Volume> asks_;
    std::mutex mutex_;
};

// 市场数据管理器
class MarketDataManager {
public:
    void subscribe(const std::string& symbol);
    void unsubscribe(const std::string& symbol);
    void handleL2Data(const L2Data& data);
    void handleTrade(const Trade& trade);
    const OrderBook& getOrderBook(const std::string& symbol) const;
    
private:
    std::map<std::string, OrderBook> orderBooks_;
    std::map<std::string, std::queue<Trade>> recentTrades_;
    std::mutex mutex_;
};

} // namespace market
} // namespace hft
