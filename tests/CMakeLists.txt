enable_testing()
find_package(GTest REQUIRED)

add_executable(unit_tests
    core/SystemTest.cpp
    core/ConfigurationTest.cpp
    market/MarketDataTest.cpp
    execution/OrderExecutionTest.cpp
    risk/RiskManagerTest.cpp
)

target_link_libraries(unit_tests
    PRIVATE
    GTest::GTest
    GTest::Main
    hft_core
)

add_test(NAME unit_tests COMMAND unit_tests)
