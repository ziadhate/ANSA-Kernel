# ==============================================================================
# ANSA Drone OS - Kernel Makefile
# ==============================================================================

# 1. Toolchain setup
CC = arm-none-eabi-gcc
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# 2. Project Files & Directories
TARGET = build/libansa_kernel.a
BUILD_DIR = build
SRCS = Sources/scheduler.c Sources/task.c Sources/kernal_queue.c startup_stm32.c
OBJS = $(patsubst Sources/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# 3. CPU Architecture Flags
CPU_FLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16

# 4. Compiler Flags
CFLAGS = $(CPU_FLAGS) -O0 -g3 -Wall -Wextra -std=gnu11 -IIncludes

# ==============================================================================
# Build Rules
# ==============================================================================

# Default target
all: $(TARGET)
	@echo "Successfully built $(TARGET)!"

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: Sources/%.c | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(AR) rcs $@ $^

# Clean up build artifacts by deleting the build folder
clean:
	@echo "Cleaning up..."
	rm -rf $(BUILD_DIR)

.PHONY: all clean