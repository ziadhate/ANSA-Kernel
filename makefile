# ==============================================================================
# ANSA Drone OS - Kernel Makefile
# ==============================================================================

# 1. Toolchain setup
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# 2. Project Files & Directories
TARGET = ansa_kernel
BUILD_DIR = build
SRCS = scheduler.c startup_stm32.c

# Prefix the build directory to the object files
OBJS = $(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))

# 3. CPU Architecture Flags
CPU_FLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16

# 4. Compiler Flags
CFLAGS = $(CPU_FLAGS) -O0 -g3 -Wall -Wextra -std=gnu11

# ==============================================================================
# Build Rules
# ==============================================================================

# Default target
all: $(BUILD_DIR) $(OBJS)
	@echo "Successfully compiled object files into $(BUILD_DIR)/!"

# Rule to create the build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# How to compile .c files into .o files inside the build folder
$(BUILD_DIR)/%.o: %.c kernel/scheduler.h | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts by deleting the whole build folder
clean:
	@echo "Cleaning up..."
	rm -rf $(BUILD_DIR)

.PHONY: all clean