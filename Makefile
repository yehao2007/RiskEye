# 编译器和选项
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
INCLUDES = -I./include -I.
LIBS = -lQt5Core -lQt5Widgets -lQt5Charts -lpthread

# 输出目录
BIN_DIR = bin
OBJ_DIR = obj

# 源文件
CORE_SRCS = $(wildcard core/*.cpp)
MARKET_SRCS = $(wildcard market/*.cpp)
EXEC_SRCS = $(wildcard execution/*.cpp)
GUI_SRCS = $(wildcard gui/*.cpp)
CLI_SRCS = $(wildcard cli/*.cpp)
UTILS_SRCS = $(wildcard utils/*.cpp)

# 目标文件
CORE_OBJS = $(CORE_SRCS:%.cpp=$(OBJ_DIR)/%.o)
MARKET_OBJS = $(MARKET_SRCS:%.cpp=$(OBJ_DIR)/%.o)
EXEC_OBJS = $(EXEC_SRCS:%.cpp=$(OBJ_DIR)/%.o)
GUI_OBJS = $(GUI_SRCS:%.cpp=$(OBJ_DIR)/%.o)
CLI_OBJS = $(CLI_SRCS:%.cpp=$(OBJ_DIR)/%.o)
UTILS_OBJS = $(UTILS_SRCS:%.cpp=$(OBJ_DIR)/%.o)

# 最终目标
TARGET = $(BIN_DIR)/HFTSystem

all: $(TARGET)

$(TARGET): $(CORE_OBJS) $(MARKET_OBJS) $(EXEC_OBJS) $(GUI_OBJS) $(CLI_OBJS) $(UTILS_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $^ -o $@ $(LIBS)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
