#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include "json/json.h"

namespace hft {
namespace core {

// 配置变更通知结构体
struct ConfigChange {
    std::string key;
    std::string oldValue;
    std::string newValue;
};

class Configuration {
public:
    Configuration();
    ~Configuration();

    // 从文件加载配置
    bool loadFromFile(const std::string& filename);
    // 从命令行参数加载配置
    bool loadFromCommandLine(int argc, char** argv);

    // 获取配置
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& key, int defaultValue = 0) const;
    double getDouble(const std::string& key, double defaultValue = 0.0) const;
    bool getBool(const std::string& key, bool defaultValue = false) const;
    std::vector<std::string> getStringList(const std::string& key) const;

    // 设置配置项
    void setString(const std::string& key, const std::string& value);
    void setInt(const std::string& key, int value);
    void setDouble(const std::string& key, double value);
    void setBool(const std::string& key, bool value);
    void setStringList(const std::string& key, const std::vector<std::string>& values);

    // 动态配置更新
    void updateConfig(const std::string& key, const Json::Value& value);

    // 添加配置变更通知
    void addConfigChangeListener(std::function<void(const ConfigChange&)> listener);

    // 验证配置
    bool validateConfig();

    // 导出配置到文件
    bool exportConfig(const std::string& filename) const;

private:
    std::unordered_map<std::string, std::string> m_stringConfig;
    std::unordered_map<std::string, int> m_intConfig;
    std::unordered_map<std::string, double> m_doubleConfig;
    std::unordered_map<std::string, bool> m_boolConfig;
    std::unordered_map<std::string, std::vector<std::string>> m_listConfig;
    std::vector<std::function<void(const ConfigChange&)>> m_listeners;
    Json::Value m_rootConfig;
    Json::Value m_schema; // 配置验证 schema
};

} // namespace core
} // namespace hft
// core/Configuration.h 改进
class Configuration {
public:
    // 添加动态配置更新
    void updateConfig(const std::string& key, const json& value);
    
    // 添加配置验证
    bool validateConfig(const ConfigSchema& schema);
    
    // 添加配置变更通知
    void addConfigChangeListener(std::function<void(const ConfigChange&)> listener);
    
private:
    nlohmann::json config_;
    // ... 其他成员
};