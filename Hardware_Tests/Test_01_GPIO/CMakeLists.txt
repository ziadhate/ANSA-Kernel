cmake_minimum_required(VERSION 3.20)

# ============================================================
# STM32L476RGTX - Test 01 GPIO
# Bare Metal
# ============================================================

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(Test_01_GPIO C ASM)

set(CMAKE_C_STANDARD 11)

# ============================================================
# CPU
# ============================================================

set(MCU_FLAGS
    -mcpu=cortex-m4
    -mthumb
    -mfloat-abi=hard
    -mfpu=fpv4-sp-d16
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
# Sources
# ============================================================

add_executable(Test_01_GPIO.elf
    main.c
    startup_stm32l476rgtx.s
)

# ============================================================
# Linker
# ============================================================

target_link_options(Test_01_GPIO.elf PRIVATE
    ${MCU_FLAGS}
    -T${CMAKE_CURRENT_SOURCE_DIR}/STM32L476RGTX_FLASH.ld
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/Test_01_GPIO.map
    --specs=nosys.specs
)

# ============================================================
# Generate HEX
# ============================================================

add_custom_command(
    TARGET Test_01_GPIO.elf POST_BUILD

    COMMAND arm-none-eabi-objcopy
            -O ihex
            $<TARGET_FILE:Test_01_GPIO.elf>
            ${CMAKE_CURRENT_BINARY_DIR}/Test_01_GPIO.hex

    COMMAND arm-none-eabi-size
            $<TARGET_FILE:Test_01_GPIO.elf>

    COMMENT "Generating HEX file..."
)
