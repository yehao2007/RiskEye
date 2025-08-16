/**
 * 🧙‍♂️ 智能配置向导 - 交互式命令行程序
 * 
 * 这个程序将引导用户通过简单的问答生成最适合的配置
 */

#include "IntelligentConfigWizard.h"
#include "ProfessionalConfigGenerator.h"
#include "../core/Logger.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <limits>

using namespace hft::ai::config;

/**
 * 🎨 控制台界面工具
 */
class ConsoleUI {
public:
    static void printHeader(const std::string& title) {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "🎯 " << title << std::endl;
        std::cout << std::string(60, '=') << std::endl;
    }
    
    static void printStep(int step, const std::string& description) {
        std::cout << "\n📋 步骤 " << step << ": " << description << std::endl;
        std::cout << std::string(50, '-') << std::endl;
    }
    
    static void printOption(int option, const std::string& text) {
        std::cout << "   " << option << ". " << text << std::endl;
    }
    
    static void printSuccess(const std::string& message) {
        std::cout << "✅ " << message << std::endl;
    }
    
    static void printWarning(const std::string& message) {
        std::cout << "⚠️  " << message << std::endl;
    }
    
    static void printError(const std::string& message) {
        std::cout << "❌ " << message << std::endl;
    }
    
    static int getIntegerInput(const std::string& prompt, int min_val, int max_val) {
        int value;
        while (true) {
            std::cout << prompt << " [" << min_val << "-" << max_val << "]: ";
            if (std::cin >> value && value >= min_val && value <= max_val) {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return value;
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            printError("请输入有效数字");
        }
    }
    
    static double getDoubleInput(const std::string& prompt, double min_val, double max_val) {
        double value;
        while (true) {
            std::cout << prompt << " [" << min_val << "-" << max_val << "]: ";
            if (std::cin >> value && value >= min_val && value <= max_val) {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return value;
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            printError("请输入有效数字");
        }
    }
    
    static bool getYesNoInput(const std::string& prompt) {
        std::string input;
        while (true) {
            std::cout << prompt << " [y/n]: ";
            std::getline(std::cin, input);
            if (input == "y" || input == "Y" || input == "yes" || input == "Yes") {
                return true;
            }
            if (input == "n" || input == "N" || input == "no" || input == "No") {
                return false;
            }
            printError("请输入 y 或 n");
        }
    }
};

/**
 * 🎯 交易场景选择
 */
TradingScenario selectTradingScenario() {
    ConsoleUI::printStep(1, "选择您的主要交易场景");
    
    std::cout << "\n请选择最符合您需求的交易场景：\n" << std::endl;
    
    ConsoleUI::printOption(1, "🏎️  超高频套利 (延迟 < 50微秒, 适合技术套利)");
    ConsoleUI::printOption(2, "🏦 量化对冲基金 (高精度, 多策略组合)");
    ConsoleUI::printOption(3, "💰 加密货币交易 (高波动, 24/7交易)");
    ConsoleUI::printOption(4, "🌍 跨市场套利 (全球市场, 货币对冲)");
    ConsoleUI::printOption(5, "⚡ 市场做市 (流动性提供, 价差收益)");
    ConsoleUI::printOption(6, "🎲 算法交易 (趋势跟踪, 均值回归)");
    ConsoleUI::printOption(7, "🛡️  风险平价基金 (风险均衡配置)");
    ConsoleUI::printOption(8, "📊 波动率交易 (期权策略, VIX交易)");
    
    int choice = ConsoleUI::getIntegerInput("\n请输入您的选择", 1, 8);
    
    switch (choice) {
        case 1: return TradingScenario::ULTRA_HFT_ARBITRAGE;
        case 2: return TradingScenario::QUANTITATIVE_HEDGE_FUND;
        case 3: return TradingScenario::CRYPTOCURRENCY_TRADING;
        case 4: return TradingScenario::CROSS_MARKET_ARBITRAGE;
        case 5: return TradingScenario::MARKET_MAKING;
        case 6: return TradingScenario::ALGORITHMIC_TRADING;
        case 7: return TradingScenario::RISK_PARITY_FUND;
        case 8: return TradingScenario::VOLATILITY_TRADING;
        default: return TradingScenario::ALGORITHMIC_TRADING;
    }
}

/**
 * ⚖️ 性能偏好配置
 */
struct PerformancePreference {
    double target_latency_us;
    double target_accuracy;
    std::string priority;
};

PerformancePreference configurePerformancePreference(TradingScenario scenario) {
    ConsoleUI::printStep(2, "配置性能偏好");
    
    PerformancePreference pref;
    
    std::cout << "\n您更看重哪个方面？\n" << std::endl;
    ConsoleUI::printOption(1, "⚡ 极致速度 (最低延迟, 适度准确率)");
    ConsoleUI::printOption(2, "🎯 平衡性能 (速度与精度并重)");
    ConsoleUI::printOption(3, "🧠 极致精度 (最高准确率, 可接受延迟)");
    
    int choice = ConsoleUI::getIntegerInput("\n请输入您的选择", 1, 3);
    
    switch (choice) {
        case 1: // 极致速度
            pref.target_latency_us = 25.0;
            pref.target_accuracy = 0.88;
            pref.priority = "speed";
            break;
        case 2: // 平衡性能
            pref.target_latency_us = 200.0;
            pref.target_accuracy = 0.93;
            pref.priority = "balanced";
            break;
        case 3: // 极致精度
            pref.target_latency_us = 2000.0;
            pref.target_accuracy = 0.97;
            pref.priority = "accuracy";
            break;
    }
    
    // 根据场景调整
    if (scenario == TradingScenario::ULTRA_HFT_ARBITRAGE) {
        pref.target_latency_us = std::min(pref.target_latency_us, 100.0);
        ConsoleUI::printWarning("超高频场景已自动调整延迟目标");
    }
    
    return pref;
}

/**
 * 💻 硬件配置
 */
HardwareProfile configureHardware() {
    ConsoleUI::printStep(3, "硬件配置");
    
    std::cout << "\n您的硬件配置类型：\n" << std::endl;
    ConsoleUI::printOption(1, "💻 个人工作站 (单机开发)");
    ConsoleUI::printOption(2, "🖥️  专业服务器 (小团队)");
    ConsoleUI::printOption(3, "🏢 企业集群 (机构级)");
    ConsoleUI::printOption(4, "☁️  云端部署 (弹性扩展)");
    ConsoleUI::printOption(5, "🔧 自定义配置");
    
    int choice = ConsoleUI::getIntegerInput("\n请输入您的选择", 1, 5);
    
    HardwareProfile hardware;
    
    switch (choice) {
        case 1: // 个人工作站
            hardware = {"PersonalWorkstation", 16, 32.0, 1, "RTX 3080", false, 10.0, 100000};
            break;
        case 2: // 专业服务器
            hardware = {"ProfessionalServer", 32, 128.0, 2, "RTX 4090", false, 1.0, 500000};
            break;
        case 3: // 企业集群
            hardware = {"EnterpriseCluster", 128, 512.0, 8, "A100", true, 0.1, 2000000};
            break;
        case 4: // 云端部署
            hardware = {"CloudDeployment", 64, 256.0, 4, "V100", false, 5.0, 1000000};
            break;
        case 5: // 自定义配置
            std::cout << "\n请输入您的硬件规格：" << std::endl;
            hardware.cpu_cores = ConsoleUI::getIntegerInput("CPU核心数", 4, 512);
            hardware.memory_gb = ConsoleUI::getDoubleInput("内存大小(GB)", 8.0, 2048.0);
            hardware.gpu_count = ConsoleUI::getIntegerInput("GPU数量", 0, 16);
            if (hardware.gpu_count > 0) {
                std::cout << "GPU型号 (RTX3080/RTX4090/A100等): ";
                std::getline(std::cin, hardware.gpu_model);
            }
            hardware.has_quantum_processor = ConsoleUI::getYesNoInput("是否有量子处理器");
            hardware.name = "CustomHardware";
            hardware.network_latency_us = 1.0;
            hardware.storage_iops = 500000;
            break;
    }
    
    return hardware;
}

/**
 * 🛡️ 风险管理配置
 */
struct RiskPreference {
    double risk_tolerance;
    bool enable_conservative_mode;
    std::vector<std::string> focus_areas;
};

RiskPreference configureRiskManagement() {
    ConsoleUI::printStep(4, "风险管理偏好");
    
    RiskPreference risk;
    
    std::cout << "\n您的风险承受能力：\n" << std::endl;
    ConsoleUI::printOption(1, "🛡️  保守型 (低风险, 稳定收益)");
    ConsoleUI::printOption(2, "⚖️  平衡型 (中等风险, 平衡收益)");
    ConsoleUI::printOption(3, "🚀 激进型 (高风险, 追求高收益)");
    
    int choice = ConsoleUI::getIntegerInput("\n请输入您的选择", 1, 3);
    
    switch (choice) {
        case 1:
            risk.risk_tolerance = 0.3;
            risk.enable_conservative_mode = true;
            break;
        case 2:
            risk.risk_tolerance = 0.6;
            risk.enable_conservative_mode = false;
            break;
        case 3:
            risk.risk_tolerance = 0.9;
            risk.enable_conservative_mode = false;
            break;
    }
    
    std::cout << "\n重点关注的风险类型（可多选）：\n" << std::endl;
    ConsoleUI::printOption(1, "市场风险");
    ConsoleUI::printOption(2, "流动性风险");
    ConsoleUI::printOption(3, "信用风险");
    ConsoleUI::printOption(4, "操作风险");
    
    if (ConsoleUI::getYesNoInput("关注市场风险")) {
        risk.focus_areas.push_back("market_risk");
    }
    if (ConsoleUI::getYesNoInput("关注流动性风险")) {
        risk.focus_areas.push_back("liquidity_risk");
    }
    if (ConsoleUI::getYesNoInput("关注信用风险")) {
        risk.focus_areas.push_back("credit_risk");
    }
    if (ConsoleUI::getYesNoInput("关注操作风险")) {
        risk.focus_areas.push_back("operational_risk");
    }
    
    return risk;
}

/**
 * 🧙‍♂️ 主向导程序
 */
json runConfigWizard() {
    ConsoleUI::printHeader("智能配置向导");
    
    std::cout << "\n欢迎使用全球顶级风险预测大模型配置向导！" << std::endl;
    std::cout << "我将引导您生成最适合您需求的配置。\n" << std::endl;
    
    try {
        // 1. 选择交易场景
        auto scenario = selectTradingScenario();
        ConsoleUI::printSuccess("交易场景选择完成");
        
        // 2. 配置性能偏好
        auto performance = configurePerformancePreference(scenario);
        ConsoleUI::printSuccess("性能偏好配置完成");
        
        // 3. 硬件配置
        auto hardware = configureHardware();
        ConsoleUI::printSuccess("硬件配置完成");
        
        // 4. 风险管理
        auto risk = configureRiskManagement();
        ConsoleUI::printSuccess("风险管理配置完成");
        
        // 5. 生成配置
        ConsoleUI::printStep(5, "生成配置");
        std::cout << "正在生成您的专属配置..." << std::endl;
        
        ProfessionalConfigGenerator generator;
        auto config = generator.generateConfig(scenario, hardware);
        
        // 应用性能偏好
        if (performance.priority == "speed") {
            config = generator.optimizeForLatency(config, performance.target_latency_us);
        } else if (performance.priority == "accuracy") {
            config = generator.optimizeForAccuracy(config, performance.target_accuracy);
        }
        
        // 应用风险配置
        auto risk_config = generator.generateRiskManagementConfig(
            scenario, risk.risk_tolerance, risk.focus_areas
        );
        config["risk_management"] = risk_config;
        
        // 添加用户偏好
        config["user_preferences"] = {
            {"scenario", static_cast<int>(scenario)},
            {"performance_priority", performance.priority},
            {"risk_tolerance", risk.risk_tolerance},
            {"hardware_profile", hardware.name}
        };
        
        ConsoleUI::printSuccess("配置生成完成！");
        
        return config;
        
    } catch (const std::exception& e) {
        ConsoleUI::printError(std::string("配置生成失败: ") + e.what());
        throw;
    }
}

/**
 * 📊 配置展示
 */
void displayConfiguration(const json& config) {
    ConsoleUI::printHeader("生成的配置详情");
    
    std::cout << "🎯 目标性能:" << std::endl;
    std::cout << "   延迟目标: " << config["target_latency_ns"].get<int64_t>() / 1000.0 << " 微秒" << std::endl;
    std::cout << "   准确率目标: " << config["target_accuracy"].get<double>() * 100 << "%" << std::endl;
    
    std::cout << "\n🧠 AI模型配置:" << std::endl;
    std::cout << "   架构: " << config["model"]["architecture"].get<std::string>() << std::endl;
    std::cout << "   隐藏层维度: " << config["model"]["hidden_dim"].get<int>() << std::endl;
    std::cout << "   网络层数: " << config["model"]["num_layers"].get<int>() << std::endl;
    std::cout << "   注意力头数: " << config["model"]["num_heads"].get<int>() << std::endl;
    
    if (config.contains("quantum") && config["quantum"]["enabled"].get<bool>()) {
        std::cout << "\n🔮 量子计算:" << std::endl;
        std::cout << "   启用状态: 已启用" << std::endl;
        std::cout << "   量子比特数: " << config["quantum"]["num_qubits"].get<int>() << std::endl;
    }
    
    std::cout << "\n⚡ 性能优化:" << std::endl;
    std::cout << "   批次大小: " << config["performance"]["batch_size"].get<int>() << std::endl;
    std::cout << "   并行工作者: " << config["performance"]["parallel_workers"].get<int>() << std::endl;
    
    if (config.contains("risk_management")) {
        std::cout << "\n🛡️ 风险管理:" << std::endl;
        std::cout << "   风险容忍度: " << config["risk_management"]["risk_tolerance"].get<double>() << std::endl;
        std::cout << "   最大组合VaR: " << config["risk_management"]["max_portfolio_var"].get<double>() * 100 << "%" << std::endl;
    }
}

/**
 * 🎯 主函数
 */
int main() {
    try {
        // 设置中文编码
        std::locale::global(std::locale(""));
        
        // 运行向导
        auto config = runConfigWizard();
        
        // 显示配置
        displayConfiguration(config);
        
        // 保存配置
        ConsoleUI::printStep(6, "保存配置");
        
        std::string filename = "my_custom_config.json";
        std::cout << "配置文件名 (默认: my_custom_config.json): ";
        std::string input;
        std::getline(std::cin, input);
        if (!input.empty()) {
            filename = input;
        }
        
        std::ofstream file(filename);
        file << std::setw(2) << config << std::endl;
        file.close();
        
        ConsoleUI::printSuccess("配置已保存到 " + filename);
        
        // 询问是否创建示例代码
        if (ConsoleUI::getYesNoInput("\n是否生成示例使用代码")) {
            std::string example_code = R"(
// 使用生成的配置创建风险预测模型
#include "GlobalRiskPredictionModel.h"
#include <fstream>

int main() {
    // 加载配置
    std::ifstream config_file(")" + filename + R"(");
    json config;
    config_file >> config;
    
    // 创建模型
    auto model = std::make_unique<GlobalRiskPredictionModel>();
    model->initialize(config);
    
    // 执行预测
    MultiModalMarketData market_data; // 填充您的市场数据
    auto prediction = model->predictRisk(
        market_data,
        RiskType::MARKET_RISK,
        PredictionHorizon::MILLISECONDS_100
    );
    
    std::cout << "风险评分: " << prediction.risk_score << std::endl;
    return 0;
}
)";
            
            std::ofstream example_file("example_usage.cpp");
            example_file << example_code;
            example_file.close();
            
            ConsoleUI::printSuccess("示例代码已保存到 example_usage.cpp");
        }
        
        std::cout << "\n🎉 配置向导完成！您现在可以使用生成的配置来创建风险预测模型。" << std::endl;
        std::cout << "\n💡 下一步建议:" << std::endl;
        std::cout << "   1. 使用生成的配置编译并测试模型" << std::endl;
        std::cout << "   2. 进行回测验证配置效果" << std::endl;
        std::cout << "   3. 根据实际表现微调参数" << std::endl;
        std::cout << "   4. 部署到生产环境" << std::endl;
        
    } catch (const std::exception& e) {
        ConsoleUI::printError("向导执行失败: " + std::string(e.what()));
        return -1;
    }
    
    return 0;
}
