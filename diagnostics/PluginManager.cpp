#include "DiagnosticPlugin.h"
#include "../core/Logger.h"
#include <algorithm>
#include <stdexcept>

namespace hft {
namespace diagnostics {

void PluginManager::loadPlugin(
    std::unique_ptr<IDiagnosticPlugin> plugin,
    const PluginConfig& config) {
    
    if (!plugin) {
        throw std::invalid_argument("Plugin instance cannot be null");
    }
    
    const auto& pluginName = config.name;
    if (plugins_.find(pluginName) != plugins_.end()) {
        throw std::runtime_error("Plugin " + pluginName + " already loaded");
    }
    
    try {
        // 初始化插件
        plugin->initialize(config);
        
        // 存储插件信息
        plugins_[pluginName] = std::move(plugin);
        configs_[pluginName] = config;
        
        // 初始化统计信息
        stats_[pluginName] = PluginStats{
            .start_time = std::chrono::system_clock::now(),
            .rules_evaluated = 0,
            .rules_triggered = 0,
            .avg_evaluation_time = 0.0
        };
        
        // 通知监听器
        for (const auto& listener : listeners_) {
            listener->onPluginLoaded(pluginName);
        }
        
        Logger::info("Plugin {} v{} loaded successfully", 
                    pluginName, config.version);
                    
    } catch (const std::exception& e) {
        throw std::runtime_error(
            "Failed to load plugin " + pluginName + ": " + e.what());
    }
}

void PluginManager::unloadPlugin(const std::string& pluginName) {
    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        throw std::runtime_error("Plugin " + pluginName + " not found");
    }
    
    try {
        // 关闭插件
        it->second->shutdown();
        
        // 移除插件
        plugins_.erase(it);
        configs_.erase(pluginName);
        stats_.erase(pluginName);
        
        // 通知监听器
        for (const auto& listener : listeners_) {
            listener->onPluginUnloaded(pluginName);
        }
        
        Logger::info("Plugin {} unloaded successfully", pluginName);
        
    } catch (const std::exception& e) {
        throw std::runtime_error(
            "Failed to unload plugin " + pluginName + ": " + e.what());
    }
}

void PluginManager::enablePlugin(const std::string& pluginName) {
    auto plugin = getPlugin(pluginName);
    if (!plugin) {
        throw std::runtime_error("Plugin " + pluginName + " not found");
    }
    
    try {
        // 配置并执行插件
        const auto& config = configs_[pluginName];
        plugin->configure(config.parameters);
        plugin->execute();
        
        Logger::info("Plugin {} enabled", pluginName);
        
    } catch (const std::exception& e) {
        throw std::runtime_error(
            "Failed to enable plugin " + pluginName + ": " + e.what());
    }
}

void PluginManager::disablePlugin(const std::string& pluginName) {
    auto plugin = getPlugin(pluginName);
    if (!plugin) {
        throw std::runtime_error("Plugin " + pluginName + " not found");
    }
    
    try {
        plugin->shutdown();
        Logger::info("Plugin {} disabled", pluginName);
        
    } catch (const std::exception& e) {
        throw std::runtime_error(
            "Failed to disable plugin " + pluginName + ": " + e.what());
    }
}

std::vector<std::string> PluginManager::getLoadedPlugins() const {
    std::vector<std::string> result;
    result.reserve(plugins_.size());
    
    for (const auto& [name, _] : plugins_) {
        result.push_back(name);
    }
    
    return result;
}

IDiagnosticPlugin* PluginManager::getPlugin(const std::string& pluginName) {
    auto it = plugins_.find(pluginName);
    return it != plugins_.end() ? it->second.get() : nullptr;
}

void PluginManager::addRuleToPlugin(
    const std::string& pluginName,
    std::unique_ptr<IDiagnosticRule> rule) {
    
    auto plugin = getPlugin(pluginName);
    if (!plugin) {
        throw std::runtime_error("Plugin " + pluginName + " not found");
    }
    
    try {
        plugin->addRule(std::move(rule));
        
        // 更新统计信息
        auto& stats = stats_[pluginName];
        stats.active_rules = plugin->getRules().size();
        
        Logger::info("Rule added to plugin {}", pluginName);
        
    } catch (const std::exception& e) {
        throw std::runtime_error(
            "Failed to add rule to plugin " + pluginName + ": " + e.what());
    }
}

void PluginManager::addEventListener(
    std::shared_ptr<IPluginEventListener> listener) {
    
    if (!listener) {
        throw std::invalid_argument("Event listener cannot be null");
    }
    
    listeners_.push_back(std::move(listener));
}

void PluginManager::removeEventListener(
    std::shared_ptr<IPluginEventListener> listener) {
    
    auto it = std::find(listeners_.begin(), listeners_.end(), listener);
    if (it != listeners_.end()) {
        listeners_.erase(it);
    }
}

void PluginManager::executeAll() {
    for (const auto& [name, plugin] : plugins_) {
        if (plugin->isEnabled()) {
            try {
                plugin->execute();
                
                // 更新统计信息
                auto& stats = stats_[name];
                stats.rules_evaluated += plugin->getRules().size();
                
            } catch (const std::exception& e) {
                Logger::error("Failed to execute plugin {}: {}", name, e.what());
                
                // 通知监听器
                for (const auto& listener : listeners_) {
                    listener->onError(name, e.what());
                }
            }
        }
    }
}

void PluginManager::shutdownAll() {
    for (const auto& [name, plugin] : plugins_) {
        try {
            plugin->shutdown();
        } catch (const std::exception& e) {
            Logger::error("Failed to shutdown plugin {}: {}", name, e.what());
        }
    }
    
    plugins_.clear();
    configs_.clear();
    stats_.clear();
    listeners_.clear();
}

PluginStats PluginManager::getPluginStats(
    const std::string& pluginName) const {
    
    auto it = stats_.find(pluginName);
    if (it == stats_.end()) {
        throw std::runtime_error("Plugin " + pluginName + " not found");
    }
    
    return it->second;
}

} // namespace diagnostics
} // namespace hft
