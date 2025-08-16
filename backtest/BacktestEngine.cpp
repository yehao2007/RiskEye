#include "BacktestEngine.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include "core/Logger.h"
#include "utils/TimeUtils.h"

namespace hft {
namespace backtest {

BacktestEngine::BacktestEngine(const core::Configuration& config)
    : m_config(config),
      m_running(false),
      m_paused(false),
      m_initial_capital(1000000.0),
      m_current_capital(m_initial_capital),
      m_commission_rate(0.001) {
}

BacktestEngine::~BacktestEngine() {
    stop();
}

bool BacktestEngine::initialize() {
    // 从配置中加载参数
    m_initial_capital = m_config.get<double>("backtest.initial_capital", 1000000.0);
    m_current_capital = m_initial_capital;
    m_commission_rate = m_config.get<double>("backtest.commission_rate", 0.001);
    
    return true;
}

void BacktestEngine::setStrategy(std::shared_ptr<strategy::Strategy> strategy) {
    m_strategy = strategy;
}

bool BacktestEngine::loadHistoricalData(const std::string& data_path) {
    std::ifstream file(data_path);
    if (!file.is_open()) {
        std::cerr << "Failed to open historical data file: " << data_path << std::endl;
        return false;
    }

    std::string line;
    // 跳过表头
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        market::MarketData data;

        // 解析CSV格式的市场数据
        // 假设格式: timestamp,symbol,open,high,low,close,volume
        std::getline(ss, token, ',');
        data.timestamp = std::stoll(token);

        std::getline(ss, token, ',');
        data.symbol = token;

        std::getline(ss, token, ',');
        data.open_price = std::stod(token);

        std::getline(ss, token, ',');
        data.high_price = std::stod(token);

        std::getline(ss, token, ',');
        data.low_price = std::stod(token);

        std::getline(ss, token, ',');
        data.close_price = std::stod(token);
        data.last_price = data.close_price;

        std::getline(ss, token, ',');
        data.volume = std::stod(token);

        m_historical_data.push_back(data);
    }

    file.close();
    std::cout << "Loaded " << m_historical_data.size() << " historical data entries" << std::endl;
    return true;
}

void BacktestEngine::run() {
    if (!m_strategy || m_historical_data.empty()) {
        std::cerr << "Strategy not set or no historical data loaded" << std::endl;
        return;
    }

    m_running = true;
    m_paused = false;
    m_performance_metrics.reset();
    m_current_capital = m_initial_capital;

    std::cout << "Starting backtest with initial capital: " << m_initial_capital << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();

    for (const auto& data : m_historical_data) {
        if (!m_running) break;
        while (m_paused) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (!m_running) break;
        }
        if (!m_running) break;

        processMarketData(data);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Backtest completed in " << duration.count() << " ms" << std::endl;
    std::cout << "Final capital: " << m_current_capital << std::endl;
    std::cout << "Total return: " << ((m_current_capital - m_initial_capital) / m_initial_capital) * 100 << "%" << std::endl;

    m_running = false;
}

void BacktestEngine::pause() {
    m_paused = true;
}

void BacktestEngine::resume() {
    m_paused = false;
}

void BacktestEngine::stop() {
    m_running = false;
    m_paused = false;
}

PerformanceMetrics BacktestEngine::getResults() const {
    return m_performance_metrics;
}

void BacktestEngine::setParameters(const std::map<std::string, double>& params) {
    // 设置回测参数
}

void BacktestEngine::processMarketData(const market::MarketData& data) {
    // 模拟市场数据更新
    if (m_strategy) {
        m_strategy->onMarketData(data);
        
        // 获取策略生成的订单
        auto orders = m_strategy->execute();
        
        // 模拟订单执行
        for (const auto& order : orders) {
            // 简单模拟订单执行，实际回测中需要更复杂的订单匹配逻辑
            double fill_price = data.last_price;
            processOrderExecution(order, fill_price);
        }
    }

    // 更新性能指标
    updatePerformanceMetrics();
}

void BacktestEngine::processOrderExecution(const execution::Order& order, double fill_price) {
    // 计算交易成本
    double commission = order.size * fill_price * m_commission_rate;
    
    // 更新资金
    if (order.side == execution::OrderSide::BUY) {
        m_current_capital -= order.size * fill_price + commission;
    } else {
        m_current_capital += order.size * fill_price - commission;
    }
    
    // 更新性能指标
    m_performance_metrics.addTrade(order.size, fill_price, commission);
}

void BacktestEngine::updatePerformanceMetrics() {
    m_performance_metrics.update(m_current_capital, m_initial_capital);
}

} // namespace backtest
} // namespace hft