#include "EnhancedDiagnostics.h"
#include "../core/Logger.h"
#include <nlohmann/json.hpp>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace hft {
namespace diagnostics {

using json = nlohmann::json;

std::string EnhancedDiagnosticReport::toJson() const {
    json j;
    
    // 系统摘要
    j["system_summary"]["status"] = system_summary.status;
    j["system_summary"]["issues"] = system_summary.issues;
    
    // 硬件健康状态
    j["hardware_health"]["cpu"] = {
        {"temperature", hardware_health.cpu.temperature},
        {"utilization", hardware_health.cpu.utilization},
        {"frequency", hardware_health.cpu.frequency},
        {"core_loads", hardware_health.cpu.core_loads}
    };
    
    // ... 其他硬件指标
    
    // 性能分析
    j["performance_analysis"] = {
        {"overall_performance", performance_analysis.overall_performance},
        {"bottlenecks", performance_analysis.bottlenecks}
    };
    
    // 性能趋势
    j["performance_trend"] = {
        {"latencies", performance_trend.latencies},
        {"throughputs", performance_trend.throughputs},
        {"success_rates", performance_trend.success_rates}
    };
    
    // 问题诊断
    json issues;
    for (const auto& issue : issue_diagnosis) {
        issues.push_back({
            {"description", issue.description},
            {"severity", static_cast<int>(issue.severity)},
            {"component", issue.component},
            {"timestamp", std::chrono::system_clock::to_time_t(issue.timestamp)}
        });
    }
    j["issue_diagnosis"] = issues;
    
    // 建议措施
    j["recommendations"] = recommendations;
    
    return j.dump(4);  // 格式化输出，缩进4个空格
}

std::string EnhancedDiagnosticReport::toHtml() const {
    std::stringstream ss;
    
    // HTML头部
    ss << "<!DOCTYPE html>\n"
       << "<html>\n"
       << "<head>\n"
       << "    <title>System Diagnostic Report</title>\n"
       << "    <style>\n"
       << "        body { font-family: Arial, sans-serif; }\n"
       << "        .section { margin: 20px; padding: 10px; border: 1px solid #ccc; }\n"
       << "        .critical { color: red; }\n"
       << "        .warning { color: orange; }\n"
       << "        .normal { color: green; }\n"
       << "    </style>\n"
       << "</head>\n"
       << "<body>\n";
       
    // 系统摘要
    ss << "<div class='section'>\n"
       << "    <h2>System Summary</h2>\n"
       << "    <p>Status: " << system_summary.status << "</p>\n"
       << "    <h3>Issues:</h3>\n"
       << "    <ul>\n";
    
    for (const auto& issue : system_summary.issues) {
        ss << "        <li>" << issue << "</li>\n";
    }
    ss << "    </ul>\n</div>\n";
    
    // 硬件健康状态
    ss << "<div class='section'>\n"
       << "    <h2>Hardware Health</h2>\n"
       << "    <h3>CPU</h3>\n"
       << "    <p>Temperature: " << hardware_health.cpu.temperature << "°C</p>\n"
       << "    <p>Utilization: " << hardware_health.cpu.utilization << "%</p>\n"
       << "    <p>Frequency: " << hardware_health.cpu.frequency << "GHz</p>\n"
       << "</div>\n";
       
    // ... 其他部分
       
    // HTML尾部
    ss << "</body>\n</html>";
    
    return ss.str();
}

std::string EnhancedDiagnosticReport::toCsv() const {
    std::stringstream ss;
    
    // CSV头部
    ss << "Timestamp,Metric,Value\n";
    
    // 性能趋势数据
    for (size_t i = 0; i < performance_trend.timestamps.size(); ++i) {
        auto timestamp = std::chrono::system_clock::to_time_t(
            performance_trend.timestamps[i]);
            
        ss << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S") << ","
           << "Latency," << performance_trend.latencies[i] << "\n";
           
        ss << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S") << ","
           << "Throughput," << performance_trend.throughputs[i] << "\n";
           
        ss << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S") << ","
           << "SuccessRate," << performance_trend.success_rates[i] << "\n";
    }
    
    return ss.str();
}

} // namespace diagnostics
} // namespace hft
