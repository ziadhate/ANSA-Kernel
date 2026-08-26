CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

TARGET = test01_gpio

CPU_FLAGS = \
	-mcpu=cortex-m3 \
	-mthumb \
	-mfloat-abi=soft

CFLAGS = \
	$(CPU_FLAGS) \
	-O0 \
	-g3 \
	-Wall \
	-Wextra \
	-ffunction-sections \
	-fdata-sections \
	-std=gnu11

LDFLAGS = \
	$(CPU_FLAGS) \
	-TSTM32F103C4_FLASH.ld \
	-nostartfiles \
	-Wl,--gc-sections \
	-Wl,-Map=$(TARGET).map

all: $(TARGET).elf $(TARGET).hex

$(TARGET).elf: main.c startup_stm32f103.c STM32F103C4_FLASH.ld
	$(CC) $(CFLAGS) \
		main.c \
		startup_stm32f103.c \
		$(LDFLAGS) \
		-o $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

	@echo ""
	@echo "=========================================="
	@echo " STM32F103C4 TEST 01 BUILD SUCCESS"
	@echo "=========================================="
	@echo ""

	$(SIZE) $(TARGET).elf

clean:
	rm -f $(TARGET).elf
	rm -f $(TARGET).hex
	rm -f $(TARGET).map

.PHONY: all clean
