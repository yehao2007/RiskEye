#include "CommandLineInterface.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "../market/MarketData.h"
#include "../execution/Order.h"
#include "../strategy/Strategy.h"

namespace hft {
namespace cli {

CommandLineInterface::CommandLineInterface(core::System* system)
    : m_system(system)
    , m_running(false) {
    registerCommands();
}

void CommandLineInterface::registerCommands() {
    m_commands = {
        {
            "help",
            "Show available commands",
            [this](const auto& args) { showHelp(); }
        },
        {
            "status",
            "Show system status",
            [this](const auto& args) { showStatus(); }
        },
        {
            "market",
            "Show market data",
            [this](const auto& args) { showMarketData(); }
        },
        {
            "positions",
            "Show current positions",
            [this](const auto& args) { showPositions(); }
        },
        {
            "orders",
            "Show active orders",
            [this](const auto& args) { showOrders(); }
        },
        {
            "order",
            "Place new order: order <buy|sell> <symbol> <quantity> <price>",
            [this](const auto& args) { handleOrder(args); }
        },
        {
            "strategy",
            "Strategy operations: strategy <list|start|stop|config> [name] [params]",
            [this](const auto& args) { handleStrategy(args); }
        },
        {
            "config",
            "Configure system: config <get|set> <key> [value]",
            [this](const auto& args) { handleConfig(args); }
        },
        {
            "exit",
            "Exit the program",
            [this](const auto& args) { m_running = false; }
        }
    };
}

void CommandLineInterface::run() {
    m_running = true;
    std::cout << "HFT System Command Line Interface" << std::endl;
    std::cout << "Type 'help' for available commands" << std::endl;
    
    std::string line;
    while (m_running && std::getline(std::cin, line)) {
        if (!line.empty()) {
            processCommand(line);
        }
        std::cout << "> ";
    }
}

void CommandLineInterface::processCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;
    
    std::vector<std::string> args;
    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }
    
    auto it = std::find_if(m_commands.begin(), m_commands.end(),
        [&cmd](const Command& c) { return c.name == cmd; });
    
    if (it != m_commands.end()) {
        it->handler(args);
    } else {
        std::cout << "Unknown command. Type 'help' for available commands." << std::endl;
    }
}

void CommandLineInterface::showHelp() {
    std::cout << "Available commands:" << std::endl;
    for (const auto& cmd : m_commands) {
        std::cout << std::setw(15) << std::left << cmd.name 
                  << cmd.description << std::endl;
    }
}

void CommandLineInterface::showStatus() {
    auto status = m_system->getStatus();
    auto metrics = m_system->getPerformanceMonitor().getMetrics();
    
    std::cout << "System Status:" << std::endl
              << "  State: " << (status == core::SystemStatus::RUNNING ? "Running" : "Stopped") << std::endl
              << "  Latency: " << metrics.averageLatency << " μs" << std::endl
              << "  Throughput: " << metrics.throughput << " ops/s" << std::endl;
}

// ... 实现其他命令处理函数 ...

} // namespace cli
} // namespace hft
