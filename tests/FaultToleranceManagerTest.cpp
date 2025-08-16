#include <gtest/gtest.h>
#include "core/FaultToleranceManager.h"
#include "core/System.h"
#include "core/AlertManager.h"
#include "network/DistributedCommunicator.h"
#include <gmock/gmock.h>

using namespace hft;
using namespace testing;

// 模拟系统组件
class MockSystem : public core::System {
public:
    MOCK_METHOD(bool, initialize, (const core::Configuration&), (override));
    MOCK_METHOD(bool, start, (), (override));
    MOCK_METHOD(void, shutdown, (), (override));
    MOCK_METHOD(core::SystemStatus, getStatus, (), (const, override));
};

// 模拟告警管理器
class MockAlertManager : public core::AlertManager {
public:
    MOCK_METHOD(bool, initialize, (), (override));
    MOCK_METHOD(void, createAlert, (const Alert&), (override));
    MOCK_METHOD(void, setAlertRule, (const AlertRule&), (override));
};

// 测试套件
class FaultToleranceManagerTest : public Test {
protected:
    void SetUp() override {
        system_ = std::make_shared<MockSystem>();
        alert_manager_ = std::make_shared<MockAlertManager>();
        communicator_ = std::make_shared<network::DistributedCommunicator>();

        // 初始化测试对象
        fault_tolerance_ = std::make_unique<core::FaultToleranceManager>(
            system_.get(), alert_manager_.get(), communicator_.get());
    }

    std::shared_ptr<MockSystem> system_;
    std::shared_ptr<MockAlertManager> alert_manager_;
    std::shared_ptr<network::DistributedCommunicator> communicator_;
    std::unique_ptr<core::FaultToleranceManager> fault_tolerance_;
};

// 测试初始化
TEST_F(FaultToleranceManagerTest, Initialize) {
    EXPECT_CALL(*alert_manager_, setAlertRule(_)).Times(1);
    EXPECT_TRUE(fault_tolerance_->initialize());
}

// 测试故障检测启动和停止
TEST_F(FaultToleranceManagerTest, StartAndStopFaultDetection) {
    fault_tolerance_->startFaultDetection();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    fault_tolerance_->stopFaultDetection();
}

// 测试组件故障处理
TEST_F(FaultToleranceManagerTest, HandleComponentFault) {
    EXPECT_CALL(*alert_manager_, createAlert(_)).Times(2); // 故障告警和恢复告警

    // 手动触发故障处理
    fault_tolerance_->handleFault("PhotonicEngine", "Connection timeout");
}

// 测试系统健康状态
TEST_F(FaultToleranceManagerTest, SystemHealth) {
    // 所有组件正常
    EXPECT_EQ(fault_tolerance_->getSystemHealth(), 1.0f);

    // 这里需要修改组件状态来测试不同健康值
    // 但因为checkComponentHealth是私有的，我们需要使用友元或其他方式
    // 实际测试中可能需要重构代码以提高可测试性
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}