#include "Configuration.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <json/json.h>

namespace hft {
namespace core {

Configuration::Configuration() {
}

Configuration::~Configuration() {
}

bool Configuration::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open configuration file: " << filename << std::endl;
        return false;
    }

    try {
        Json::Value root;
        file >> root;

        // 加载字符串配置
        if (root.isMember("string_configs")) {
            const Json::Value& stringConfigs = root["string_configs"];
            for (Json::Value::const_iterator it = stringConfigs.begin(); it != stringConfigs.end(); ++it) {
                std::string key = it.key().asString();
                m_stringConfig[key] = it->asString();
            }
        }

        // 加载整数配置
        if (root.isMember("int_configs")) {
            const Json::Value& intConfigs = root["int_configs"];
            for (Json::Value::const_iterator it = intConfigs.begin(); it != intConfigs.end(); ++it) {
                std::string key = it.key().asString();
                m_intConfig[key] = it->asInt();
            }
        }

        // 加载浮点数配置
        if (root.isMember("double_configs")) {
            const Json::Value& doubleConfigs = root["double_configs"];
            for (Json::Value::const_iterator it = doubleConfigs.begin(); it != doubleConfigs.end(); ++it) {
                std::string key = it.key().asString();
                m_doubleConfig[key] = it->asDouble();
            }
        }

        // 加载布尔值配置
        if (root.isMember("bool_configs")) {
            const Json::Value& boolConfigs = root["bool_configs"];
            for (Json::Value::const_iterator it = boolConfigs.begin(); it != boolConfigs.end(); ++it) {
                std::string key = it.key().asString();
                m_boolConfig[key] = it->asBool();
            }
        }

        // 加载字符串列表配置
        if (root.isMember("list_configs")) {
            const Json::Value& listConfigs = root["list_configs"];
            for (Json::Value::const_iterator it = listConfigs.begin(); it != listConfigs.end(); ++it) {
                std::string key = it.key().asString();
                const Json::Value& list = *it;
                std::vector<std::string> values;
                for (const Json::Value& item : list) {
                    values.push_back(item.asString());
                }
                m_listConfig[key] = values;
            }
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing configuration file: " << e.what() << std::endl;
        return false;
    }
}

bool Configuration::loadFromCommandLine(int argc, char** argv) {
    // 简单实现，实际应用中可能需要使用更复杂的命令行解析库
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.substr(0, 2) == "--") {
            size_t pos = arg.find('=');
            if (pos != std::string::npos) {
                std::string key = arg.substr(2, pos - 2);
                std::string value = arg.substr(pos + 1);
                m_stringConfig[key] = value;
            }
        }
    }
    return true;
}

std::string Configuration::getString(const std::string& key, const std::string& defaultValue) const {
    auto it = m_stringConfig.find(key);
    if (it != m_stringConfig.end()) {
        return it->second;
    }
    return defaultValue;
}

int Configuration::getInt(const std::string& key, int defaultValue) const {
    auto it = m_intConfig.find(key);
    if (it != m_intConfig.end()) {
        return it->second;
    }
    return defaultValue;
}

double Configuration::getDouble(const std::string& key, double defaultValue) const {
    auto it = m_doubleConfig.find(key);
    if (it != m_doubleConfig.end()) {
        return it->second;
    }
    return defaultValue;
}

bool Configuration::getBool(const std::string& key, bool defaultValue) const {
    auto it = m_boolConfig.find(key);
    if (it != m_boolConfig.end()) {
        return it->second;
    }
    return defaultValue;
}

std::vector<std::string> Configuration::getStringList(const std::string& key) const {
    auto it = m_listConfig.find(key);
    if (it != m_listConfig.end()) {
        return it->second;
    }
    return std::vector<std::string>();
}

void Configuration::setString(const std::string& key, const std::string& value) {
    m_stringConfig[key] = value;
}

void Configuration::setInt(const std::string& key, int value) {
    m_intConfig[key] = value;
}

void Configuration::setDouble(const std::string& key, double value) {
    m_doubleConfig[key] = value;
}

void Configuration::setBool(const std::string& key, bool value) {
    m_boolConfig[key] = value;
}

void Configuration::setStringList(const std::string& key, const std::vector<std::string>& values) {
    m_listConfig[key] = values;
}

void Configuration::updateConfig(const std::string& key, const Json::Value& value) {
    ConfigChange change;
    change.key = key;

    // 保存旧值
    if (m_rootConfig.isMember(key)) {
        change.oldValue = m_rootConfig[key].asString();
    } else {
        change.oldValue = "";
    }

    // 更新配置
    m_rootConfig[key] = value;
    change.newValue = value.asString();

    // 更新对应的映射
    if (value.isString()) {
        m_stringConfig[key] = value.asString();
    } else if (value.isInt()) {
        m_intConfig[key] = value.asInt();
    } else if (value.isDouble()) {
        m_doubleConfig[key] = value.asDouble();
    } else if (value.isBool()) {
        m_boolConfig[key] = value.asBool();
    } else if (value.isArray()) {
        std::vector<std::string> values;
        for (const Json::Value& item : value) {
            values.push_back(item.asString());
        }
        m_listConfig[key] = values;
    }

    // 通知所有监听器
    for (const auto& listener : m_listeners) {
        listener(change);
    }
}

void Configuration::addConfigChangeListener(std::function<void(const ConfigChange&)> listener) {
    m_listeners.push_back(listener);
}

bool Configuration::validateConfig() {
    // 实现配置验证逻辑
    return true;
}

bool Configuration::exportConfig(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file for export: " << filename << std::endl;
        return false;
    }

    try {
        file << m_rootConfig;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error exporting configuration: " << e.what() << std::endl;
        return false;
    }
}
} // namespace core
} // namespace hft