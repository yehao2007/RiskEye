#include <gtest/gtest.h>
#include "../core/System.h"
#include "../core/Configuration.h"

class SystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_.loadFromFile("test_config.json");
        system_ = std::make_unique<hft::core::System>();
    }

    hft::core::Configuration config_;
    std::unique_ptr<hft::core::System> system_;
};

TEST_F(SystemTest, InitializationSuccess) {
    EXPECT_TRUE(system_->initialize(config_));
}

TEST_F(SystemTest, GracefulShutdown) {
    ASSERT_TRUE(system_->initialize(config_));
    system_->gracefulShutdown(std::chrono::seconds(1));
    EXPECT_EQ(system_->getStatus(), SystemStatus::STOPPED);
}
