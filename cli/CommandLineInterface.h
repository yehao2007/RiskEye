#pragma once

#include <string>
#include <vector>
#include <functional>
#include "../core/System.h"

namespace hft {
namespace cli {

class CommandLineInterface {
public:
    explicit CommandLineInterface(core::System* system);
    
    void run();
    void processCommand(const std::string& command);
    
private:
    void registerCommands();
    void showHelp();
    void showStatus();
    void showMarketData();
    void showPositions();
    void showOrders();
    void handleOrder(const std::vector<std::string>& args);
    void handleStrategy(const std::vector<std::string>& args);
    void handleConfig(const std::vector<std::string>& args);
    
    struct Command {
        std::string name;
        std::string description;
        std::function<void(const std::vector<std::string>&)> handler;
    };
    
    core::System* m_system;
    std::vector<Command> m_commands;
    bool m_running;
};

} // namespace cli
} // namespace hft
