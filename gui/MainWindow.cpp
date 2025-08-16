#include "MainWindow.h"
#include <QApplication>
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

MarketDataWidget::MarketDataWidget(QWidget* parent) : QWidget(parent) {
    // 初始化订单簿表格
    m_order_book_table = new QTableWidget(10, 3, this);
    m_order_book_table->setHorizontalHeaderLabels({"价格", "数量", "类型"});

    // 初始化价格图表
    m_price_chart = new QChart();
    m_price_series = new QSplineSeries();
    m_price_chart->addSeries(m_price_series);

    m_x_axis = new QValueAxis();
    m_y_axis = new QValueAxis();
    m_price_chart->addAxis(m_x_axis, Qt::AlignBottom);
    m_price_chart->addAxis(m_y_axis, Qt::AlignLeft);
    m_price_series->attachAxis(m_x_axis);
    m_price_series->attachAxis(m_y_axis);

    QChartView* chart_view = new QChartView(m_price_chart);

    // 布局
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(new QLabel("订单簿"));
    layout->addWidget(m_order_book_table);
    layout->addWidget(new QLabel("价格走势"));
    layout->addWidget(chart_view);

    setLayout(layout);
}

void MarketDataWidget::updateMarketData(const market::MarketData& data) {
    // 更新订单簿数据
    m_order_book_table->clearContents();

    // 添加入场数据
    int row = 0;
    for (const auto& ask : data.asks) {
        if (row >= m_order_book_table->rowCount()) break;
        m_order_book_table->setItem(row, 0, new QTableWidgetItem(QString::number(ask.price)));
        m_order_book_table->setItem(row, 1, new QTableWidgetItem(QString::number(ask.size)));
        m_order_book_table->setItem(row, 2, new QTableWidgetItem("卖单"));
        row++;
    }

    // 添加买单数据
    for (const auto& bid : data.bids) {
        if (row >= m_order_book_table->rowCount()) break;
        m_order_book_table->setItem(row, 0, new QTableWidgetItem(QString::number(bid.price)));
        m_order_book_table->setItem(row, 1, new QTableWidgetItem(QString::number(bid.size)));
        m_order_book_table->setItem(row, 2, new QTableWidgetItem("买单"));
        row++;
    }

    // 更新价格图表 - 增强版本
    static double time = 0;
    m_price_series->append(time, data.lastPrice);
    time += 1.0;

    // 添加均线
    if (m_ma_series && m_price_history.size() >= m_ma_period) {
        double sum = std::accumulate(m_price_history.end() - m_ma_period, m_price_history.end(), 0.0);
        double ma = sum / m_ma_period;
        m_ma_series->append(time, ma);
    }

    // 保持图表最新
    m_x_axis->setRange(time - 10.0, time + 1.0);
    double min_price = data.lastPrice * 0.999;
    double max_price = data.lastPrice * 1.001;
    m_y_axis->setRange(min_price, max_price);
}

OrderManagementWidget::OrderManagementWidget(QWidget* parent) : QWidget(parent) {
    m_instrument_edit = new QLineEdit("AAPL");
    m_price_edit = new QLineEdit("150.0");
    m_size_edit = new QLineEdit("10");
    m_buy_button = new QPushButton("买入");
    m_sell_button = new QPushButton("卖出");

    // 初始化订单表格
    m_order_table = new QTableWidget(10, 5, this);
    m_order_table->setHorizontalHeaderLabels({"合约", "价格", "数量", "方向", "状态"});

    // 布局
    QHBoxLayout* input_layout = new QHBoxLayout();
    input_layout->addWidget(new QLabel("合约:"));
    input_layout->addWidget(m_instrument_edit);
    input_layout->addWidget(new QLabel("价格:"));
    input_layout->addWidget(m_price_edit);
    input_layout->addWidget(new QLabel("数量:"));
    input_layout->addWidget(m_size_edit);
    input_layout->addWidget(m_buy_button);
    input_layout->addWidget(m_sell_button);

    QVBoxLayout* main_layout = new QVBoxLayout();
    main_layout->addLayout(input_layout);
    main_layout->addWidget(new QLabel("订单列表"));
    main_layout->addWidget(m_order_table);

    setLayout(main_layout);

    // 连接信号槽
    connect(m_buy_button, &QPushButton::clicked, this, &OrderManagementWidget::sendOrder);
    connect(m_sell_button, &QPushButton::clicked, this, &OrderManagementWidget::sendOrder);
}

void OrderManagementWidget::updateOrderStatus(const execution::Order& order) {
    // 更新订单状态
    for (int i = 0; i < m_order_table->rowCount(); i++) {
        if (m_order_table->item(i, 0)->text() == QString::fromStdString(order.symbol) &&
            m_order_table->item(i, 1)->text().toDouble() == order.price &&
            m_order_table->item(i, 2)->text().toInt() == order.size) {
            m_order_table->setItem(i, 4, new QTableWidgetItem(
                execution::orderStatusToString(order.status).c_str()));
            return;
        }
    }

    // 如果是新订单，添加到表格
    int row = m_order_table->rowCount();
    m_order_table->insertRow(row);
    m_order_table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(order.symbol)));
    m_order_table->setItem(row, 1, new QTableWidgetItem(QString::number(order.price)));
    m_order_table->setItem(row, 2, new QTableWidgetItem(QString::number(order.size)));
    m_order_table->setItem(row, 3, new QTableWidgetItem(
        order.side == execution::OrderSide::BUY ? "买入" : "卖出"));
    m_order_table->setItem(row, 4, new QTableWidgetItem(
        execution::orderStatusToString(order.status).c_str()));
}

void OrderManagementWidget::sendOrder() {
    QPushButton* sender_button = qobject_cast<QPushButton*>(sender());
    if (!sender_button) return;

    execution::Order order;
    order.symbol = m_instrument_edit->text().toStdString();
    order.price = m_price_edit->text().toDouble();
    order.size = m_size_edit->text().toInt();
    order.side = (sender_button == m_buy_button) ? execution::OrderSide::BUY : execution::OrderSide::SELL;
    order.type = execution::OrderType::LIMIT;
    order.status = execution::OrderStatus::NEW;

    // 发送订单（在实际应用中，这里会调用订单执行系统）
    updateOrderStatus(order);
}

StrategyWidget::StrategyWidget(QWidget* parent) : QWidget(parent) {
    m_strategy_combo = new QComboBox();
    m_strategy_combo->addItem("趋势跟踪策略");
    m_strategy_combo->addItem("均值回归策略");
    m_strategy_combo->addItem("做市策略");

    m_parameter_table = new QTableWidget(5, 2, this);
    m_parameter_table->setHorizontalHeaderLabels({"参数", "值"});

    m_start_button = new QPushButton("启动策略");
    m_stop_button = new QPushButton("停止策略");

    // 布局
    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->addWidget(m_strategy_combo);
    button_layout->addWidget(m_start_button);
    button_layout->addWidget(m_stop_button);

    QVBoxLayout* main_layout = new QVBoxLayout();
    main_layout->addLayout(button_layout);
    main_layout->addWidget(new QLabel("策略参数"));
    main_layout->addWidget(m_parameter_table);

    setLayout(main_layout);

    // 连接信号槽
    connect(m_strategy_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &StrategyWidget::selectStrategy);
    connect(m_start_button, &QPushButton::clicked, this, &StrategyWidget::startStrategy);
    connect(m_stop_button, &QPushButton::clicked, this, &StrategyWidget::stopStrategy);

    // 初始化显示第一个策略的参数
    selectStrategy(0);
}

void StrategyWidget::updateStrategyParameters() {
    // 更新策略参数（在实际应用中，这里会调用策略系统）
}

void StrategyWidget::selectStrategy(int index) {
    // 清空表格
    m_parameter_table->clearContents();

    // 根据选择的策略显示不同的参数
    if (index == 0) { // 趋势跟踪策略
        m_parameter_table->setRowCount(3);
        m_parameter_table->setItem(0, 0, new QTableWidgetItem("周期"));
        m_parameter_table->setItem(0, 1, new QTableWidgetItem("20"));
        m_parameter_table->setItem(1, 0, new QTableWidgetItem("阈值"));
        m_parameter_table->setItem(1, 1, new QTableWidgetItem("0.02"));
        m_parameter_table->setItem(2, 0, new QTableWidgetItem("仓位限制"));
        m_parameter_table->setItem(2, 1, new QTableWidgetItem("100"));
    } else if (index == 1) { // 均值回归策略
        m_parameter_table->setRowCount(3);
        m_parameter_table->setItem(0, 0, new QTableWidgetItem("周期"));
        m_parameter_table->setItem(0, 1, new QTableWidgetItem("30"));
        m_parameter_table->setItem(1, 0, new QTableWidgetItem("标准差"));
        m_parameter_table->setItem(1, 1, new QTableWidgetItem("2.0"));
        m_parameter_table->setItem(2, 0, new QTableWidgetItem("仓位限制"));
        m_parameter_table->setItem(2, 1, new QTableWidgetItem("100"));
    } else if (index == 2) { // 做市策略
        m_parameter_table->setRowCount(3);
        m_parameter_table->setItem(0, 0, new QTableWidgetItem("价差"));
        m_parameter_table->setItem(0, 1, new QTableWidgetItem("0.01"));
        m_parameter_table->setItem(1, 0, new QTableWidgetItem("深度"));
        m_parameter_table->setItem(1, 1, new QTableWidgetItem("5"));
        m_parameter_table->setItem(2, 0, new QTableWidgetItem("仓位限制"));
        m_parameter_table->setItem(2, 1, new QTableWidgetItem("200"));
    }
}

void StrategyWidget::startStrategy() {
    // 启动策略（在实际应用中，这里会调用策略系统）
    m_start_button->setEnabled(false);
    m_stop_button->setEnabled(true);
}

void StrategyWidget::stopStrategy() {
    // 停止策略（在实际应用中，这里会调用策略系统）
    m_start_button->setEnabled(true);
    m_stop_button->setEnabled(false);
}

RiskManagementWidget::RiskManagementWidget(QWidget* parent) : QWidget(parent) {
    m_max_position_edit = new QLineEdit("1000");
    m_max_loss_edit = new QLineEdit("5000");
    m_risk_threshold_edit = new QLineEdit("0.05");

    m_risk_metrics_table = new QTableWidget(5, 2, this);
    m_risk_metrics_table->setHorizontalHeaderLabels({"指标", "值"});

    // 布局
    QGridLayout* input_layout = new QGridLayout();
    input_layout->addWidget(new QLabel("最大仓位:"), 0, 0);
    input_layout->addWidget(m_max_position_edit, 0, 1);
    input_layout->addWidget(new QLabel("每日最大亏损:"), 1, 0);
    input_layout->addWidget(m_max_loss_edit, 1, 1);
    input_layout->addWidget(new QLabel("风险阈值:"), 2, 0);
    input_layout->addWidget(m_risk_threshold_edit, 2, 1);

    QVBoxLayout* main_layout = new QVBoxLayout();
    main_layout->addLayout(input_layout);
    main_layout->addWidget(new QPushButton("更新风险限制"));
    main_layout->addWidget(new QLabel("风险指标"));
    main_layout->addWidget(m_risk_metrics_table);

    setLayout(main_layout);
}

void RiskManagementWidget::updateRiskLimits() {
    // 更新风险限制（在实际应用中，这里会调用风险管理系统）
}

void RiskManagementWidget::updateRiskMetrics(const risk::RiskMetrics& metrics) {
    // 更新风险指标
    m_risk_metrics_table->setItem(0, 0, new QTableWidgetItem("当前仓位"));
    m_risk_metrics_table->setItem(0, 1, new QTableWidgetItem(QString::number(metrics.currentPosition)));

    m_risk_metrics_table->setItem(1, 0, new QTableWidgetItem("当日盈亏"));
    m_risk_metrics_table->setItem(1, 1, new QTableWidgetItem(QString::number(metrics.dailyPnl)));

    m_risk_metrics_table->setItem(2, 0, new QTableWidgetItem("VaR(95%)"));
    m_risk_metrics_table->setItem(2, 1, new QTableWidgetItem(QString::number(metrics.var95)));

    m_risk_metrics_table->setItem(3, 0, new QTableWidgetItem("夏普比率"));
    m_risk_metrics_table->setItem(3, 1, new QTableWidgetItem(QString::number(metrics.sharpeRatio)));

    m_risk_metrics_table->setItem(4, 0, new QTableWidgetItem("风险分数"));
    m_risk_metrics_table->setItem(4, 1, new QTableWidgetItem(QString::number(metrics.riskScore)));
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_dark_mode(false) {
    setWindowTitle("高频交易系统");
    setMinimumSize(1024, 768);

    // 创建中央部件
    QWidget* central_widget = new QWidget();
    setCentralWidget(central_widget);

    // 创建标签页控件
    m_tab_widget = new QTabWidget();

    // 创建各个标签页
    m_market_data_widget = new MarketDataWidget();
    m_order_management_widget = new OrderManagementWidget();
    m_strategy_widget = new StrategyWidget();
    m_risk_management_widget = new RiskManagementWidget();

    // 添加标签页
    m_tab_widget->addTab(m_market_data_widget, "市场数据");
    m_tab_widget->addTab(m_order_management_widget, "订单管理");
    m_tab_widget->addTab(m_strategy_widget, "策略");
    m_tab_widget->addTab(m_risk_management_widget, "风险管理");

    // 创建状态栏控件
    m_connect_button = new QPushButton("连接");
    m_theme_button = new QPushButton("切换主题");
    m_status_label = new QLabel("未连接");

    // 布局
    QHBoxLayout* status_layout = new QHBoxLayout();
    status_layout->addWidget(m_connect_button);
    status_layout->addWidget(m_theme_button);
    status_layout->addWidget(m_status_label);
    status_layout->addStretch();

    QVBoxLayout* main_layout = new QVBoxLayout(central_widget);
    main_layout->addWidget(m_tab_widget);
    main_layout->addLayout(status_layout);

    // 连接信号槽
    connect(m_connect_button, &QPushButton::clicked, this, &MainWindow::connectToSystem);
    connect(m_theme_button, &QPushButton::clicked, this, &MainWindow::toggleDarkMode);

    // 初始化定时器
    m_update_timer = new QTimer(this);
    connect(m_update_timer, &QTimer::timeout, this, &MainWindow::updateStatus);
}

void MainWindow::toggleDarkMode() {
    m_dark_mode = !m_dark_mode;
    switchTheme(m_dark_mode);
}

void MainWindow::switchTheme(bool darkMode) {
    // 应用程序样式
    QString styleSheet;

    if (darkMode) {
        // 暗黑主题
        styleSheet = ""
            "QMainWindow, QWidget { background-color: #1e1e1e; color: #ffffff; }"
            "QTabWidget::pane { background-color: #2d2d2d; border: 1px solid #444444; }"
            "QTabBar::tab { background-color: #333333; color: #ffffff; padding: 6px 12px; }"
            "QTabBar::tab:selected { background-color: #2d2d2d; border-bottom: 2px solid #0078d7; }"
            "QPushButton { background-color: #333333; color: #ffffff; border: 1px solid #444444; padding: 4px 8px; }"
            "QPushButton:hover { background-color: #444444; }"
            "QLineEdit { background-color: #333333; color: #ffffff; border: 1px solid #444444; padding: 4px; }"
            "QTableWidget { background-color: #2d2d2d; color: #ffffff; border: 1px solid #444444; }"
            "QHeaderView::section { background-color: #333333; color: #ffffff; border: 1px solid #444444; }"
            "QLabel { color: #ffffff; }";
    } else {
        // 亮色主题
        styleSheet = ""
            "QMainWindow, QWidget { background-color: #f0f0f0; color: #000000; }"
            "QTabWidget::pane { background-color: #ffffff; border: 1px solid #cccccc; }"
            "QTabBar::tab { background-color: #e0e0e0; color: #000000; padding: 6px 12px; }"
            "QTabBar::tab:selected { background-color: #ffffff; border-bottom: 2px solid #0078d7; }"
            "QPushButton { background-color: #e0e0e0; color: #000000; border: 1px solid #cccccc; padding: 4px 8px; }"
            "QPushButton:hover { background-color: #d0d0d0; }"
            "QLineEdit { background-color: #ffffff; color: #000000; border: 1px solid #cccccc; padding: 4px; }"
            "QTableWidget { background-color: #ffffff; color: #000000; border: 1px solid #cccccc; }"
            "QHeaderView::section { background-color: #e0e0e0; color: #000000; border: 1px solid #cccccc; }"
            "QLabel { color: #000000; }";
    }

    // 应用样式
    qApp->setStyleSheet(styleSheet);
}

void MainWindow::updateUI() {
    // 获取最新市场数据并更新UI
    auto market_data = m_system->getMarketDataFeed()->getLastMarketData("AAPL");
    if (market_data) {
        m_market_data_widget->updateMarketData(*market_data);
    }

    // 获取最新风险指标并更新UI
    auto risk_metrics = m_system->getRiskManager()->getRiskMetrics();
    m_risk_management_widget->updateRiskMetrics(risk_metrics);

    // 更新订单状态
    // 注意：在实际应用中，这里应该通过回调或事件来更新，而不是轮询
}

} // namespace gui
} // namespace hft