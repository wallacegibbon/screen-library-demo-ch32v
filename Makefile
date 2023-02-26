CROSS_TOOLCHAIN_PATH = $(HOME)/MRS_Toolchain_Linux_x64_V1.60
CROSS_COMPILER_PREFIX = $(CROSS_TOOLCHAIN_PATH)/RISC-V Embedded GCC/bin/riscv-none-embed-
OPENOCD_PATH = $(CROSS_TOOLCHAIN_PATH)/OpenOCD

TARGET_NAME = main
BUILD_DIR = build

STDPERIPH_DIR = $(HOME)/CH32V103StdPeripheral
ARCH = -march=rv32imac -mabi=ilp32
#ARCH = -march=rv32ec -mabi=ilp32e

CROSS_CC = "$(CROSS_COMPILER_PREFIX)gcc"
CROSS_LD = "$(CROSS_COMPILER_PREFIX)gcc"
CROSS_GDB = "$(CROSS_COMPILER_PREFIX)gdb"
CROSS_OBJCOPY = "$(CROSS_COMPILER_PREFIX)objcopy"
CROSS_OBJDUMP = "$(CROSS_COMPILER_PREFIX)objdump"
CROSS_SIZE = "$(CROSS_COMPILER_PREFIX)size"
OPENOCD = "$(OPENOCD_PATH)/bin/openocd"
OPENOCD_ARGS = "-f $(OPENOCD_PATH)/bin/wch-riscv.cfg"

CROSS_C_SOURCE_FILES = $(wildcard ./src/*.c)
CROSS_C_SOURCE_FILES += $(wildcard ./src/screen-library-mcu/common/src/*.c)
CROSS_C_SOURCE_FILES += $(wildcard ./src/screen-library-mcu/ch32v103/src/*.c)
CROSS_C_SOURCE_FILES += $(wildcard $(STDPERIPH_DIR)/src/*.c)

CROSS_ASM_SOURCE_FILES = ./start.S

CROSS_OBJECTS += $(addprefix $(BUILD_DIR)/, $(notdir $(CROSS_C_SOURCE_FILES:.c=.c.o)))
CROSS_OBJECTS += $(addprefix $(BUILD_DIR)/, $(notdir $(CROSS_ASM_SOURCE_FILES:.S=.S.o)))

vpath %.c $(sort $(dir $(CROSS_C_SOURCE_FILES)))
vpath %.S $(sort $(dir $(CROSS_ASM_SOURCE_FILES)))

CROSS_C_ASM_FLAGS = \
$(ARCH) -W -g -Os \
-ffunction-sections -fdata-sections -fno-common -fno-builtin \
-I$(STDPERIPH_DIR)/inc \
-I./src \
-I./src/screen-library-mcu/common/inc \
-I./src/screen-library-mcu/ch32v103/inc \

CROSS_LD_FLAGS = \
-T./link.ld $(ARCH) -nostartfiles -specs=nano.specs \
-Wl,--gc-sections \
-Wl,--no-relax \
-Wl,-Map=$(BUILD_DIR)/$(TARGET_NAME).map,--cref \

all: $(BUILD_DIR)/$(TARGET_NAME).hex $(BUILD_DIR)/$(TARGET_NAME).bin

$(BUILD_DIR)/$(TARGET_NAME).hex: $(BUILD_DIR)/$(TARGET_NAME).elf
	@echo "\tGenerating hex and bin files..."
	@$(CROSS_OBJCOPY) -Obinary $< $(TARGET_NAME).bin
	@$(CROSS_OBJCOPY) -Oihex $< $(TARGET_NAME).hex
	@echo "\n\tMemory Usage:\n"
	@$(CROSS_SIZE) $<
	@echo "\n\tdone.\n"

$(BUILD_DIR)/$(TARGET_NAME).elf: $(CROSS_OBJECTS)
	@echo "\n\tLinking..."
	@$(CROSS_LD) $(CROSS_LD_FLAGS) -o $@ $^

$(BUILD_DIR)/%.c.o: %.c | $(BUILD_DIR)
	@echo "\tCompiling $<..."
	@$(CROSS_CC) $(CROSS_C_ASM_FLAGS) -MMD -MF"$(@:%.o=%.d)" -c -o $@ $<

$(BUILD_DIR)/%.S.o: %.S | $(BUILD_DIR)
	@echo "\tCompiling $<..."
	@$(CROSS_CC) $(CROSS_C_ASM_FLAGS) -MMD -MF"$(@:%.o=%.d)" -c -o $@ $<

$(BUILD_DIR):
	@mkdir $@

DEPENDENCY_FILES = $(patsubst %.S, %.S.d, $(ASM_FILES))
DEPENDENCY_FILES += $(patsubst %.c, %.c.d, $(C_FILES))
-include $(DEPENDENCY_FILES)

.PHONY: load openocd debug lss tags clean

load:
	@echo "\tLoad program to the target machine..."
	@$(OPENOCD) $(OPENOCD_ARGS) -d1 \
		-c "program $(TARGET_NAME).hex verify reset exit"

openocd:
	@echo "\tStarting the OpenOCD server..."
	@$(OPENOCD) $(OPENOCD_ARGS)

debug:
	@echo "\tStarting GDB and connect to OpenOCD..."
	@$(CROSS_GDB) $(TARGET_NAME).elf \
		--eval-command="target extended-remote localhost:3333"

tags:
	@echo "\tGenerating ctag file..."
	@find $(LIBDIR) . -regex '.*\.[ch]' -exec realpath {} \; \
		| sort | uniq | xargs ctags

lss:
	@echo "\tGenerating disassembled file..."
	@$(CROSS_OBJDUMP) -S -D $(TARGET_NAME).elf > $(TARGET_NAME).lss

clean:
	@echo "\tRemoving generated files..."
	@rm -rf $(BUILD_DIR)

