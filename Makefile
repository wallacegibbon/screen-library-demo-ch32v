CROSS_COMPILER_PREFIX = $(HOME)/MRS_Toolchain_Linux_x64_V1.60/RISC-V Embedded GCC/bin/riscv-none-embed-
OPENOCD = /usr/local/bin/openocd
OPENOCD_ARGS = -f interface/wlink.cfg -f target/wch-riscv.cfg

ARCH = -march=rv32imac -mabi=ilp32
#ARCH = -march=rv32ec -mabi=ilp32e

LIB_PERIPHERAL_DIR = $(HOME)/CH32_standard_peripheral_library/ch32v10x

CROSS_C_SOURCE_FILES += $(wildcard $(LIB_PERIPHERAL_DIR)/src/*.c)
CROSS_C_SOURCE_FILES += $(wildcard ./src/screen-library-mcu/*.c)
CROSS_C_SOURCE_FILES += $(wildcard ./src/screen-library-mcu/ch32v10x/*.c)
CROSS_C_SOURCE_FILES += $(wildcard ./src/*.c)

CROSS_ASM_SOURCE_FILES = $(wildcard ./src/*.S)

CROSS_LINKER_SCRIPT = ./src/ch32v103r8t6.ld

CROSS_C_ASM_INCLUDES = \
-I$(LIB_PERIPHERAL_DIR)/inc \
-I./src/screen-library-mcu/ch32v10x -I./src/screen-library-mcu -I./src \

OPENOCD_FLASH_COMMANDS = \
-c "program $<" -c wlink_reset_resume -c exit

include ./miscellaneous-makefiles/cross-gcc-mcu.mk

