#pragma once
#include <string>
#include <functional>
#include <unordered_map>
#include <memory>
#include "Strategy.h"

namespace hft {
namespace strategy {

class StrategyFactory {
public:
    // 注册策略创建函数
    static void registerStrategy(const std::string& name, 
                                std::function<std::shared_ptr<Strategy>()> creator);

    // 创建策略
    static std::shared_ptr<Strategy> createStrategy(const std::string& name);

    // 获取所有可用策略名称
    static std::vector<std::string> getAvailableStrategies();

private:
    static std::unordered_map<std::string, std::function<std::shared_ptr<Strategy>()>> m_strategy_creators;
};

// 策略注册宏
#define REGISTER_STRATEGY(class_name) \
    class class_name##_Registrar { \
    public: \
        class_name##_Registrar() { \
            StrategyFactory::registerStrategy(#class_name, []() { \
                return std::make_shared<class_name>(); \
            }); \
        } \
    }; \
    static class_name##_Registrar g_##class_name##_registrar;

} // namespace strategy
} // namespace hft


#pragma once
#include <string>
#include <functional>
#include <unordered_map>
#include <memory>
#include "Strategy.h"

namespace hft {
namespace strategy {

class StrategyFactory {
public:
    // 注册策略创建函数
    static void registerStrategy(const std::string& name, 
                                std::function<std::shared_ptr<Strategy>()> creator);

    // 创建策略
    static std::shared_ptr<Strategy> createStrategy(const std::string& name);

    // 获取所有可用策略名称
    static std::vector<std::string> getAvailableStrategies();

private:
    static std::unordered_map<std::string, std::function<std::shared_ptr<Strategy>()>> m_strategy_creators;
};

// 策略注册宏
#define REGISTER_STRATEGY(class_name) \
    class class_name##_Registrar { \
    public: \
        class_name##_Registrar() { \
            StrategyFactory::registerStrategy(#class_name, []() { \
                return std::make_shared<class_name>(); \
            }); \
        } \
    }; \
    static class_name##_Registrar g_##class_name##_registrar;

} // namespace strategy
} // namespace hft