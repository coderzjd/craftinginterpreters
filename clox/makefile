#---------------------------------
# clox  – 不污染源码目录 / 自动扫源 / 一键运行
#---------------------------------
CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -O2 -Wno-unused-parameter
LDFLAGS := -lm

SRCS    := $(wildcard *.c)
OBJS    := $(patsubst %.c,build/%.o,$(SRCS))
TARGET  := build/clox

run: all
	@./$(TARGET) lox.lox
repl: all
	@./${TARGET}

all: $(TARGET)

build:
	@mkdir -p build

# | build
# 只要目标文件已存在，就完全忽略它的时间戳；
# 只有当目标文件“压根不存在”时，才执行一次 order-only 依赖对应的规则。
# 每次有人往 build/ 里写文件，目录的时间戳也会更新，
# 导致所有 .o 都会被判定为“比目录旧”，下一次 make 就会全员重新编译——完全没必要
$(TARGET): $(OBJS) | build
	@$(CC) $^ -o $@ $(LDFLAGS)

build/%.o: %.c | build
	@$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS += -g -DDEBUG
debug: clean all

clean:
	@rm -rf build

# PHONY 的核心作用只有一句话：告诉 make“all / clean / debug 这些名字根本不是文件，你别费劲去磁盘上找它们，更别因为‘某个文件恰好叫这个名字’就跳过规则”
.PHONY: all clean debug run