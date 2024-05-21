CROSS_COMPILER_PREFIX = C:/MounRiver/MounRiver_Studio/toolchain/RISC-V Embedded GCC/bin/riscv-none-embed-
#CROSS_COMPILER_PREFIX = $(HOME)/MRS_Toolchain_Linux_x64_V1.80/RISC-V Embedded GCC/bin/riscv-none-embed-

OPENOCD = C:/MounRiver/MounRiver_Studio/toolchain/OpenOCD/bin/openocd.exe
OPENOCD_ARGS = -f C:/MounRiver/MounRiver_Studio/toolchain/OpenOCD/bin/wch-riscv.cfg
#OPENOCD = $(HOME)/MRS_Toolchain_Linux_x64_V1.80/OpenOCD/bin/openocd
#OPENOCD_ARGS = -f $(HOME)/MRS_Toolchain_Linux_x64_V1.80/OpenOCD/bin/wch-riscv.cfg
#OPENOCD = /usr/local/bin/openocd
#OPENOCD_ARGS = -f interface/wlink.cfg -f target/wch-riscv.cfg

ARCH = -march=rv32imafc -mabi=ilp32f
#ARCH = -march=rv32imac -mabi=ilp32
#ARCH = -march=rv32ec -mabi=ilp32e

CH32_STD_LIB_DIR = ../miscellaneous-periph-libraries/ch32v30x

CROSS_C_SOURCE_FILES += $(wildcard $(CH32_STD_LIB_DIR)/peripheral/src/*.c)
CROSS_C_SOURCE_FILES += $(wildcard $(CH32_STD_LIB_DIR)/core/*.c)
CROSS_C_SOURCE_FILES += $(wildcard ../generic-screen-painter/src/*.c)
CROSS_C_SOURCE_FILES += ../generic-screen-painter/adaptor/ch32v/sc_ssd1306_ch32v_i2c.c
CROSS_C_SOURCE_FILES += ../generic-screen-painter/adaptor/ch32v/sc_common_ch32v.c
CROSS_C_SOURCE_FILES += ../generic-screen-painter/adaptor/ch32v/sc_st7789_ch32v_fsmc.c
CROSS_C_SOURCE_FILES += ./src/camera_ov2640.c
CROSS_C_SOURCE_FILES += ./src/ch32v_debug.c
CROSS_C_SOURCE_FILES += ./src/ch32v_it.c
CROSS_C_SOURCE_FILES += ./src/main.c

CROSS_ASM_SOURCE_FILES += $(CH32_STD_LIB_DIR)/sample/startup.S

CROSS_C_INCLUDES = $(CH32_STD_LIB_DIR)/peripheral/inc $(CH32_STD_LIB_DIR)/core \
../generic-screen-painter/include ../generic-screen-painter/adaptor/ch32v ./src \

CROSS_C_FLAGS += -fno-common -fno-builtin -Os
CROSS_C_FLAGS += -DCHIP_CH32V30X

#CROSS_C_FLAGS += -DBEZIER_DEBUG

CROSS_LD_FLAGS += -Wl,--no-relax -specs=nosys.specs -specs=nano.specs -nostartfiles \
-T$(CH32_STD_LIB_DIR)/sample/default.ld

CROSS_LD_FLAGS += -lm

OPENOCD_FLASH_COMMANDS = -c "program $< verify" -c wlink_reset_resume -c exit

include ./cross-gcc-mcu.mk

