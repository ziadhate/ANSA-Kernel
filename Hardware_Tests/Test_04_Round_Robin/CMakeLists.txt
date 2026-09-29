cmake_minimum_required(VERSION 3.20)

# ============================================================
# STM32F103C4 - Round Robin Hardware Test
# Cortex-M3
# ============================================================

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(Test_04_Round_Robin C ASM)

set(CMAKE_C_STANDARD 11)

# ============================================================
# CPU
# STM32F103C4 = Cortex-M3
# ============================================================

set(MCU_FLAGS
    -mcpu=cortex-m3
    -mthumb
)

# ============================================================
# Compiler
# ============================================================

add_compile_options(
    ${MCU_FLAGS}
    -O0
    -g3
    -Wall
    -Wextra
    -ffunction-sections
    -fdata-sections
)

# ============================================================
# Include directories
# ============================================================

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Port/Cortex_M4/Inc
)

# ============================================================
# Sources
# ============================================================

add_executable(Test_04_Round_Robin.elf

    main.c

    startup_stm32f103.c

    ../../Src/scheduler.c
    ../../Src/task.c
    ../../Src/queue.c
    ../../Src/mutex.c
    ../../Src/semaphore.c
    ../../Src/watchdog.c
    ../../Src/watchdog_port.c

    ../../Port/Cortex_M4/Src/port.c
)

# ============================================================
# Test-specific small-memory configuration
# STM32F103C4 = 6 KB RAM
# ============================================================

target_compile_definitions(Test_04_Round_Robin.elf PRIVATE
    MAX_TASKS=6
    KERNEL_STACK_POOL_SIZE=1024U
    TASK_STACK_MIN_WORDS=32U
)

# ============================================================
# Linker
# ============================================================

target_link_options(Test_04_Round_Robin.elf PRIVATE

    ${MCU_FLAGS}

    -T${CMAKE_CURRENT_SOURCE_DIR}/STM32F103C4_FLASH.ld

    -Wl,--gc-sections

    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/Test_04_Round_Robin.map

    --specs=nosys.specs
)

# ============================================================
# Generate HEX
# ============================================================

add_custom_command(
    TARGET Test_04_Round_Robin.elf POST_BUILD

    COMMAND arm-none-eabi-objcopy
            -O ihex
            $<TARGET_FILE:Test_04_Round_Robin.elf>
            ${CMAKE_CURRENT_BINARY_DIR}/Test_04_Round_Robin.hex

    COMMAND arm-none-eabi-size
            $<TARGET_FILE:Test_04_Round_Robin.elf>

    COMMENT "Generating HEX file..."
)
