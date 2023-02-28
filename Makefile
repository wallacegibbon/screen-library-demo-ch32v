CROSS_COMPILER_PREFIX = $(HOME)/MRS_Toolchain_Linux_x64_V1.60/RISC-V Embedded GCC/bin/riscv-none-embed-
OPENOCD_PATH = $(HOME)/MRS_Toolchain_Linux_x64_V1.60/OpenOCD

BUILD_DIR = build
TARGET = app

STDPERIPH_DIR = $(HOME)/CH32_standard_peripherals_library/CH32V103
ARCH = -march=rv32imac -mabi=ilp32
#ARCH = -march=rv32ec -mabi=ilp32e

CROSS_C_SOURCE_FILES += $(wildcard $(STDPERIPH_DIR)/src/*.c)
CROSS_C_SOURCE_FILES += $(wildcard ./src/screen-library-mcu/*.c)
CROSS_C_SOURCE_FILES += $(wildcard ./src/screen-library-mcu/ch32v103/*.c)
CROSS_C_SOURCE_FILES += $(wildcard ./src/*.c)

CROSS_ASM_SOURCE_FILES = $(wildcard ./*.S)

CROSS_OBJECTS += $(addprefix $(BUILD_DIR)/, $(notdir $(CROSS_C_SOURCE_FILES:.c=.c.o)))
CROSS_OBJECTS += $(addprefix $(BUILD_DIR)/, $(notdir $(CROSS_ASM_SOURCE_FILES:.S=.S.o)))

vpath %.c $(sort $(dir $(CROSS_C_SOURCE_FILES)))
vpath %.S $(sort $(dir $(CROSS_ASM_SOURCE_FILES)))

CROSS_C_ASM_FLAGS = $(ARCH) -W -g -Os -ffunction-sections -fdata-sections \
-fno-common -fno-builtin \
-I$(STDPERIPH_DIR)/inc \
-I./src/screen-library-mcu/ch32v103 -I./src/screen-library-mcu -I./src \

CROSS_LD_FLAGS = $(ARCH) -T./link.ld -nostartfiles -specs=nosys.specs \
-Wl,--gc-sections -Wl,--no-relax -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref \

CROSS_CC = "$(CROSS_COMPILER_PREFIX)gcc"
CROSS_LD = "$(CROSS_COMPILER_PREFIX)gcc"
CROSS_GDB = "$(CROSS_COMPILER_PREFIX)gdb"
CROSS_OBJCOPY = "$(CROSS_COMPILER_PREFIX)objcopy"
CROSS_OBJDUMP = "$(CROSS_COMPILER_PREFIX)objdump"
CROSS_SIZE = "$(CROSS_COMPILER_PREFIX)size"
OPENOCD = "$(OPENOCD_PATH)/bin/openocd"
OPENOCD_ARGS = -f "$(OPENOCD_PATH)/bin/wch-riscv.cfg"

all: $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR)/%.c.o: %.c | $(BUILD_DIR)
	@echo "\tCC $< ..."
	@$(CROSS_CC) $(CROSS_C_ASM_FLAGS) -MMD -MF"$(@:%.o=%.d)" -c -o $@ $<

$(BUILD_DIR)/%.S.o: %.S | $(BUILD_DIR)
	@echo "\tAS $< ..."
	@$(CROSS_CC) $(CROSS_C_ASM_FLAGS) -MMD -MF"$(@:%.o=%.d)" -c -o $@ $<

$(BUILD_DIR)/$(TARGET).elf: $(CROSS_OBJECTS)
	@echo "\tLD $@ ..."
	@$(CROSS_LD) $(CROSS_LD_FLAGS) -o $@ $^
	@$(CROSS_OBJDUMP) -S -D $@ > $@.lss
	@echo
	@echo "\tMemory Usage:"
	@$(CROSS_SIZE) $@
	@echo

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	@echo "\tGenerating HEX file ..."
	@$(CROSS_OBJCOPY) -Oihex $< $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	@echo "\tGenerating BIN file ..."
	@$(CROSS_OBJCOPY) -Obinary $< $@

$(BUILD_DIR):
	@mkdir $@

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: flash openocd debug clean

flash:
	@$(OPENOCD) $(OPENOCD_ARGS) -c init -c halt \
		-c "flash erase_sector wch_riscv 0 last" \
		-c "program $(BUILD_DIR)/$(TARGET).hex" \
		-c wlink_reset_resume -c exit

openocd:
	@$(OPENOCD) $(OPENOCD_ARGS)

debug:
	@$(CROSS_GDB) $(BUILD_DIR)/$(TARGET).elf \
		--eval-command="target extended-remote localhost:3333"

clean:
	@rm -rf $(BUILD_DIR)

