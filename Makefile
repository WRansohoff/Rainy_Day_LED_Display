# Makefile for compiling ARM Cortex-M assembly projects.
TARGET = main

# Default to M0 QFP32 8KB SRAM, 64KB Flash.
#MCU ?= STM32F051K8
# Or M0 QFP32 4KB SRAM, 32KB Flash.
#MCU ?= STM32F030K6
# Or M0 TSSOP20 4KB SRAM, 32KB Flash.
MCU ?= STM32F031F6
# Or, M4F QFP32 12KB SRAM, 64KB Flash.
#MCU ?= STM32F303K8

# Linker scripts for memory allocation.
ifeq ($(MCU), STM32F051K8)
	CHIP_FILE = STM32F051K8T6
	MCU_CLASS = F0
	MCU_PERIPH_CLASS = STM32F051
	MCU_HAL_CLASS = STM32F051x8
else ifeq ($(MCU), STM32F030F4)
	CHIP_FILE = STM32F030F4T6
	MCU_CLASS = F0
	MCU_PERIPH_CLASS = STM32F030
	MCU_HAL_CLASS = STM32F030x4
else ifeq ($(MCU), STM32F031F6)
	CHIP_FILE = STM32F031F6T6
	MCU_CLASS = F0
	MCU_PERIPH_CLASS = STM32F031
	MCU_HAL_CLASS = STM32F031x6
else ifeq ($(MCU), STM32F030K6)
	CHIP_FILE = STM32F030K6T6
	MCU_CLASS = F0
	MCU_PERIPH_CLASS = STM32F030
	MCU_HAL_CLASS = STM32F030x6
else ifeq ($(MCU), STM32L051K8)
	# TODO: Support the L0-series.
	CHIP_FILE = STM32L051K8T6
	MCU_CLASS = L0
	MCU_PERIPH_CLASS = STM32L051
	MCU_HAL_CLASS = STM32L051x8
else ifeq ($(MCU), STM32F303K8)
	CHIP_FILE = STM32F303K8T6
	MCU_CLASS = F3
	MCU_PERIPH_CLASS = STM32F303
	MCU_HAL_CLASS = STM32F303x8
endif

LD_SCRIPT = ${CHIP_FILE}.ld
VECT_TBL  = ./vector_tables/${CHIP_FILE}_vt.S
BOOT_FILE = ./boot_s/${CHIP_FILE}_boot.S

ifeq ($(MCU_CLASS), F0)
	MCU_SPEC = cortex-m0
else ifeq ($(MCU_CLASS), L0)
	MCU_SPEC = cortex-m0plus
else ifeq ($(MCU_CLASS), F3)
	MCU_SPEC = cortex-m4
endif

# Toolchain definitions (ARM bare metal defaults)
TOOLCHAIN = /usr
CC = $(TOOLCHAIN)/bin/arm-none-eabi-gcc
AS = $(TOOLCHAIN)/bin/arm-none-eabi-as
LD = $(TOOLCHAIN)/bin/arm-none-eabi-ld
OC = $(TOOLCHAIN)/bin/arm-none-eabi-objcopy
OD = $(TOOLCHAIN)/bin/arm-none-eabi-objdump
OS = $(TOOLCHAIN)/bin/arm-none-eabi-size

# Assembly directives.
ASFLAGS += -mcpu=$(MCU_SPEC)
ASFLAGS += -mthumb
ASFLAGS += -Wall
#ASFLAGS += -c
# (Set error messages to appear on a single line.)
ASFLAGS += -fmessage-length=0
# Custom flags for preprocessor definitions:
ASFLAGS += -D$(MCU)
ASFLAGS += -DVVC_$(MCU_CLASS)
ASFLAGS += -DVVC_$(CHIP_FILE)

# C compilation directives
CFLAGS += -mcpu=$(MCU_SPEC)
CFLAGS += -mthumb
CFLAGS += -Wall
CFLAGS += -g
# (Set error messages to appear on a single line.)
CFLAGS += -fmessage-length=0
# (Set system to ignore semihosted junk)
CFLAGS += --specs=nosys.specs
# Custom flags for preprocessor definitions:
CFLAGS += -D$(MCU)
CFLAGS += -DVVC_$(MCU_CLASS)
CFLAGS += -DUSE_FULL_LL_DRIVER
CFLAGS += -D$(MCU_PERIPH_CLASS)
CFLAGS += -D$(MCU_HAL_CLASS)

# Linker directives.
LSCRIPT = ./ld/$(LD_SCRIPT)
LFLAGS += -mcpu=$(MCU_SPEC)
LFLAGS += -mthumb
LFLAGS += -Wall
LFLAGS += --specs=nosys.specs
LFLAGS += -nostdlib
LFLAGS += -lgcc
#LFLAGS += -nostartfiles
#LFLAGS += -static
LFLAGS += -T$(LSCRIPT)

AS_SRC += ./src/core.S
AS_SRC += ./src/util.S
AS_SRC += $(VECT_TBL)
AS_SRC += $(BOOT_FILE)

C_SRC  += ./src/main.c
C_SRC  += ./src/util_c.c
C_SRC  += ./src/interrupts_c.c
# HAL and LL libs.
ifeq ($(MCU_CLASS), F0)
	C_SRC  += ./src/device_headers/system_stm32f0xx.c
	C_SRC  += ./src/hal_libs_f0/stm32f0xx_ll_gpio.c
	C_SRC  += ./src/hal_libs_f0/stm32f0xx_ll_rcc.c
	C_SRC  += ./src/hal_libs_f0/stm32f0xx_ll_tim.c
else ifeq ($(MCU_CLASS), L0)
	# TODO
else ifeq ($(MCU_CLASS), F3)
	C_SRC  += ./src/hal_libs_f3/stm32f3xx_ll_gpio.c
	C_SRC  += ./src/hal_libs_f3/stm32f3xx_ll_rcc.c
	C_SRC  += ./src/hal_libs_f3/stm32f3xx_ll_tim.c
endif

# (other header file directories, if any)
INCLUDE =   -I./src
INCLUDE +=  -I./src/device_headers
ifeq ($(MCU_CLASS), F0)
	INCLUDE +=  -I./src/hal_libs_f0
else ifeq ($(MCU_CLASS), L0)
	# TODO
else ifeq ($(MCU_CLASS), F3)
	INCLUDE +=  -I./src/hal_libs_f3
endif

OBJS =  $(AS_SRC:.S=.o)
OBJS += $(C_SRC:.c=.o)

.PHONY: all
all: $(TARGET).bin

%.o: %.S
	$(CC) -x assembler-with-cpp -c -O0 $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

$(TARGET).elf: $(OBJS)
	$(CC) $^ $(LFLAGS) -o $@

$(TARGET).bin: $(TARGET).elf
	$(OC) -S -O binary $< $@
	$(OS) $<

.PHONY: clean
clean:
	rm -f $(OBJS)
	rm -f ./vector_tables/*.o
	rm -f ./boot_s/*.o
	rm -f $(TARGET).elf $(TARGET).bin
