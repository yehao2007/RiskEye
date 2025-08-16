#pragma once

#include <string>
#include <vector>
#include <memory>
#include <regex>
#include "../core/Logger.h"

namespace hft {
namespace core {

class LogAnalyzer {
public:
    // 日志级别
    enum class LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        CRITICAL
    };

    // 日志模式
    struct LogPattern {
        std::string pattern;
        LogLevel level;
        std::string category;
        std::vector<std::string> tags;
        std::function<void(const LogEvent&)> callback;
    };

    // 日志事件
    struct LogEvent {
        uint64_t timestamp;
        LogLevel level;
        std::string message;
        std::string component;
        std::string thread_id;
        std::unordered_map<std::string, std::string> attributes;
        std::vector<std::string> stack_trace;
    };

    // 初始化分析器
    bool initialize(const std::string& config_path = "") {
        try {
            // 初始化存储引擎
            initializeStorage();
            
            // 加载分析规则
            loadAnalysisRules();
            
            // 启动实时分析
            startRealTimeAnalysis();
            
            return true;
        } catch (const std::exception& e) {
            Logger::error("Log analyzer initialization failed: {}", e.what());
            return false;
        }
    }

    // 分析日志事件
    void analyzeEvent(const LogEvent& event) {
        // 更新统计信息
        updateStats(event);
        
        // 模式匹配
        matchPatterns(event);
        
        // 异常检测
        detectAnomalies(event);
        
        // 性能分析
        analyzePerformance(event);
    }

    // 设置分析规则
    void addAnalysisPattern(const LogPattern& pattern) {
        patterns_.push_back(pattern);
    }

    // 获取分析结果
    struct AnalysisResult {
        std::vector<std::string> detected_patterns;
        std::vector<std::string> anomalies;
        std::vector<std::string> performance_issues;
        std::unordered_map<std::string, uint64_t> event_counts;
        double error_rate;
        std::vector<std::string> recommendations;
    };
    AnalysisResult getResults() const;

    // 导出分析报告
    void exportAnalysisReport(const std::string& filename);

private:
    // 存储引擎
    class StorageEngine {
    public:
        void store(const LogEvent& event);
        std::vector<LogEvent> query(const std::string& filter);
        void optimize();
        
    private:
        struct LogIndex {
            std::unordered_map<std::string, std::vector<uint64_t>> component_index;
            std::unordered_map<LogLevel, std::vector<uint64_t>> level_index;
            std::unordered_map<std::string, std::vector<uint64_t>> pattern_index;
        };
        LogIndex index_;
        std::vector<LogEvent> events_;
    };

    // 模式匹配引擎
    class PatternMatcher {
    public:
        void addPattern(const LogPattern& pattern);
        std::vector<std::string> match(const LogEvent& event);
        
    private:
        std::vector<LogPattern> patterns_;
        std::vector<std::regex> compiled_patterns_;
    };

    // 异常检测引擎
    class AnomalyDetector {
    public:
        void train(const std::vector<LogEvent>& history);
        std::vector<std::string> detect(const LogEvent& event);
        
    private:
        // 统计模型
        struct StatModel {
            double mean;
            double std_dev;
            std::vector<double> history;
        };
        std::unordered_map<std::string, StatModel> models_;
    };

    // 性能分析引擎
    class PerformanceAnalyzer {
    public:
        void analyze(const LogEvent& event);
        std::vector<std::string> getIssues();
        
    private:
        struct LatencyStats {
            double p50;
            double p95;
            double p99;
            std::vector<double> samples;
        };
        std::unordered_map<std::string, LatencyStats> latency_stats_;
    };

private:
    std::unique_ptr<StorageEngine> storage_;
    std::unique_ptr<PatternMatcher> matcher_;
    std::unique_ptr<AnomalyDetector> detector_;
    std::unique_ptr<PerformanceAnalyzer> perf_analyzer_;
    
    std::vector<LogPattern> patterns_;
    std::atomic<uint64_t> total_events_{0};
    std::atomic<uint64_t> error_events_{0};
    
    // 配置参数
    struct Config {
        size_t max_history_size = 1000000;
        bool enable_real_time = true;
        std::chrono::seconds analysis_interval{1};
        bool detailed_performance = true;
    } config_;
};

} // namespace core
} // namespace hft
