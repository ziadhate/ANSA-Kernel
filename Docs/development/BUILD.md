# Build and Development

## Build Requirements

The project requires an ARM embedded GCC toolchain for supported
Cortex-M targets.

Typical tools include:

- arm-none-eabi-gcc
- arm-none-eabi-ar
- arm-none-eabi-objcopy
- make

## Build

The kernel build system is defined by the repository Makefile.

Before building, verify that the ARM toolchain is available:

    arm-none-eabi-gcc --version

Then use the repository build instructions.

## Development Rule

Changes to kernel behavior should be accompanied by appropriate
verification or tests.
