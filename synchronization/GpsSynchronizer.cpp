#include "GpsSynchronizer.h"
#include "TimeProvider.h"
#include <windows.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <chrono>

namespace hft {
namespace synchronization {

GpsSynchronizer::GpsSynchronizer(const std::string& devicePath) 
    : m_devicePath(devicePath), m_deviceHandle(-1),
      m_running(false), m_synchronized(false) {
}

GpsSynchronizer::~GpsSynchronizer() {
    stop();
    if (m_deviceHandle != -1) {
        closeDevice();
    }
}

bool GpsSynchronizer::initialize() {
    if (!openDevice()) {
        return false;
    }

    m_timeProvider.initialize();
    return true;
}

void GpsSynchronizer::start() {
    if (m_running) {
        return;
    }

    m_running = true;
    m_syncThread = std::thread(&GpsSynchronizer::syncThreadFunc, this);
}

void GpsSynchronizer::stop() {
    if (!m_running) {
        return;
    }

    m_running = false;
    if (m_syncThread.joinable()) {
        m_syncThread.join();
    }
    m_synchronized = false;
}

uint64_t GpsSynchronizer::getCurrentNanos() const {
    if (!m_synchronized) {
        throw std::runtime_error("GPS not synchronized");
    }
    return m_timeProvider.getCurrentNanos();
}

std::string GpsSynchronizer::getCurrentUtcTime() const {
    if (!m_synchronized) {
        throw std::runtime_error("GPS not synchronized");
    }
    return m_timeProvider.getCurrentUtcTime();
}

void GpsSynchronizer::syncThreadFunc() {
    char buffer[1024];
    std::string gpsData;

    while (m_running) {
        // 模拟从GPS设备读取数据
        // 实际应用中应使用ReadFile或类似函数从串口读取
        DWORD bytesRead = 0;
        if (m_deviceHandle != -1) {
            // 这里只是模拟，实际应用中需要根据GPS设备接口实现
            Sleep(100); // 模拟读取延迟
            snprintf(buffer, sizeof(buffer), "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A\r\n");
            bytesRead = strlen(buffer);
            gpsData.append(buffer, bytesRead);

            // 解析GPS数据
            if (parseGpsData(gpsData)) {
                m_synchronized = true;
            }
        } else {
            Sleep(100);
        }
    }
}

bool GpsSynchronizer::parseGpsData(const std::string& data) {
    // 查找GPRMC语句
    size_t pos = data.find("$GPRMC");
    if (pos == std::string::npos) {
        return false;
    }

    // 提取GPRMC语句
    size_t end = data.find("\r\n", pos);
    if (end == std::string::npos) {
        return false;
    }

    std::string gprmc = data.substr(pos, end - pos);

    // 解析GPRMC语句
    // $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
    // 格式: 时间,状态,纬度,北纬/南纬,经度,东经/西经,速度,航向,日期,磁偏角,东/西,校验和
    std::istringstream iss(gprmc);
    std::string token;
    std::vector<std::string> tokens;

    while (std::getline(iss, token, ',')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 10) {
        return false;
    }

    // 检查状态是否有效
    if (tokens[2] != "A") {
        return false;
    }

    // 解析时间 (hhmmss)
    std::string timeStr = tokens[1];
    if (timeStr.length() != 6) {
        return false;
    }

    // 解析日期 (ddmmyy)
    std::string dateStr = tokens[9];
    if (dateStr.length() != 6) {
        return false;
    }

    // 构建UTC时间字符串
    std::string utcTime = "19" + dateStr.substr(4, 2) + "-" + dateStr.substr(2, 2) + "-" + dateStr.substr(0, 2) + "T" + 
                          timeStr.substr(0, 2) + ":" + timeStr.substr(2, 2) + ":" + timeStr.substr(4, 2) + "Z";

    // 将UTC时间转换为纳秒并更新时间提供器
    uint64_t nanos = m_timeProvider.utcTimeToNanos(utcTime);
    m_timeProvider.updateCurrentTime(nanos);

    return true;
}

bool GpsSynchronizer::openDevice() {
    // 打开GPS设备（串口）
    HANDLE hComm = CreateFileA(
        m_devicePath.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hComm == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open GPS device: " << GetLastError() << std::endl;
        return false;
    }

    // 配置串口
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hComm, &dcbSerialParams)) {
        std::cerr << "Failed to get comm state: " << GetLastError() << std::endl;
        CloseHandle(hComm);
        return false;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hComm, &dcbSerialParams)) {
        std::cerr << "Failed to set comm state: " << GetLastError() << std::endl;
        CloseHandle(hComm);
        return false;
    }

    // 配置超时
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hComm, &timeouts)) {
        std::cerr << "Failed to set comm timeouts: " << GetLastError() << std::endl;
        CloseHandle(hComm);
        return false;
    }

    m_deviceHandle = (int)(uintptr_t)hComm;
    return true;
}

void GpsSynchronizer::closeDevice() {
    if (m_deviceHandle != -1) {
        CloseHandle((HANDLE)(uintptr_t)m_deviceHandle);
        m_deviceHandle = -1;
    }
}

} // namespace synchronization
} // namespace hft