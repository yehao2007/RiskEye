#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
#include <QChart>
#include <QChartView>
#include <QSplineSeries>
#include <QValueAxis>
#include <QTimer>
#include <memory>
#include "core/System.h"
#include "strategy/StrategyManager.h"
#include "execution/OrderExecution.h"

namespace hft {
namespace gui {

class MarketDataWidget : public QWidget {
    Q_OBJECT

public:
    MarketDataWidget(QWidget* parent = nullptr);

public slots:
    void updateMarketData(const market::MarketData& data);
    void toggleTechnicalIndicator(bool enabled);
    void changeTimePeriod(int period);

private:
    QTableWidget* m_order_book_table;
    QChart* m_price_chart;
    QSplineSeries* m_price_series;
    QSplineSeries* m_ma_series;
    QSplineSeries* m_ema_series;
    QSplineSeries* m_macd_series;
    QValueAxis* m_x_axis;
    QValueAxis* m_y_axis;
    QValueAxis* m_macd_axis;
    std::vector<double> m_price_history;
    int m_ma_period;
    int m_ema_period;
};

class OrderManagementWidget : public QWidget {
    Q_OBJECT

public:
    OrderManagementWidget(QWidget* parent = nullptr);

public slots:
    void updateOrderStatus(const execution::Order& order);
    void sendOrder();

private:
    QLineEdit* m_instrument_edit;
    QLineEdit* m_price_edit;
    QLineEdit* m_size_edit;
    QPushButton* m_buy_button;
    QPushButton* m_sell_button;
    QTableWidget* m_order_table;
};

class StrategyWidget : public QWidget {
    Q_OBJECT

public:
    StrategyWidget(QWidget* parent = nullptr);

public slots:
    void updateStrategyParameters();
    void selectStrategy(int index);
    void startStrategy();
    void stopStrategy();

private:
    QComboBox* m_strategy_combo;
    QTableWidget* m_parameter_table;
    QPushButton* m_start_button;
    QPushButton* m_stop_button;
};

class RiskManagementWidget : public QWidget {
    Q_OBJECT

public:
    RiskManagementWidget(QWidget* parent = nullptr);

public slots:
    void updateRiskLimits();
    void updateRiskMetrics(const risk::RiskMetrics& metrics);

private:
    QLineEdit* m_max_position_edit;
    QLineEdit* m_max_loss_edit;
    QLineEdit* m_risk_threshold_edit;
    QTableWidget* m_risk_metrics_table;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void setSystem(std::shared_ptr<core::System> system);
    void switchTheme(bool darkMode);

public slots:
    void connectToSystem();
    void disconnectFromSystem();
    void updateStatus();
    void toggleDarkMode();

private:
    std::shared_ptr<core::System> m_system;
    QTabWidget* m_tab_widget;
    MarketDataWidget* m_market_data_widget;
    OrderManagementWidget* m_order_management_widget;
    StrategyWidget* m_strategy_widget;
    RiskManagementWidget* m_risk_management_widget;
    QPushButton* m_connect_button;
    QPushButton* m_theme_button;
    QLabel* m_status_label;
    QTimer* m_update_timer;
    bool m_dark_mode;
};

} // namespace gui
} // namespace hft
class MarketDataWidget : public QWidget {
    Q_OBJECT

public:
    MarketDataWidget(QWidget* parent = nullptr);

public slots:
    void updateMarketData(const market::MarketData& data);
    void toggleTechnicalIndicator(bool enabled);
    void changeTimePeriod(int period);

private:
    QTableWidget* m_order_book_table;
    QChart* m_price_chart;
    QSplineSeries* m_price_series;
    QSplineSeries* m_ma_series;
    QSplineSeries* m_ema_series;
    QSplineSeries* m_macd_series;
    QValueAxis* m_x_axis;
    QValueAxis* m_y_axis;
    QValueAxis* m_macd_axis;
    std::vector<double> m_price_history;
    int m_ma_period;
    int m_ema_period;
};

class OrderManagementWidget : public QWidget {
    Q_OBJECT

public:
    OrderManagementWidget(QWidget* parent = nullptr);

public slots:
    void updateOrderStatus(const execution::Order& order);
    void sendOrder();

private:
    QLineEdit* m_instrument_edit;
    QLineEdit* m_price_edit;
    QLineEdit* m_size_edit;
    QPushButton* m_buy_button;
    QPushButton* m_sell_button;
    QTableWidget* m_order_table;
};

class StrategyWidget : public QWidget {
    Q_OBJECT

public:
    StrategyWidget(QWidget* parent = nullptr);

public slots:
    void updateStrategyParameters();
    void selectStrategy(int index);
    void startStrategy();
    void stopStrategy();

private:
    QComboBox* m_strategy_combo;
    QTableWidget* m_parameter_table;
    QPushButton* m_start_button;
    QPushButton* m_stop_button;
};

class RiskManagementWidget : public QWidget {
    Q_OBJECT

public:
    RiskManagementWidget(QWidget* parent = nullptr);

public slots:
    void updateRiskLimits();
    void updateRiskMetrics(const risk::RiskMetrics& metrics);

private:
    QLineEdit* m_max_position_edit;
    QLineEdit* m_max_loss_edit;
    QLineEdit* m_risk_threshold_edit;
    QTableWidget* m_risk_metrics_table;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void setSystem(std::shared_ptr<core::System> system);
    void switchTheme(bool darkMode);

public slots:
    void connectToSystem();
    void disconnectFromSystem();
    void updateStatus();
    void toggleDarkMode();

private:
    std::shared_ptr<core::System> m_system;
    QTabWidget* m_tab_widget;
    MarketDataWidget* m_market_data_widget;
    OrderManagementWidget* m_order_management_widget;
    StrategyWidget* m_strategy_widget;
    RiskManagementWidget* m_risk_management_widget;
    QPushButton* m_connect_button;
    QPushButton* m_theme_button;
    QLabel* m_status_label;
    QTimer* m_update_timer;
    bool m_dark_mode;
};

} // namespace gui
} // namespace hft