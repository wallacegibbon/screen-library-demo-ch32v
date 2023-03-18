## Introduction

This is a demo of using my [screen library](https://github.com/wallacegibbon/screen-library-mcu) for micro-controllers.

## Notes

Currently, This [customized OpenOCD](https://github.com/karlp/openocd-hacks) is being used.

If you use the OpenOCD in MounRiver, you would need to customize the Makefile like this:

```makefile
OPENOCD_PATH = $(HOME)/MRS_Toolchain_Linux_x64_V1.60/OpenOCD
OPENOCD = "$(OPENOCD_PATH)/bin/openocd"
OPENOCD_ARGS = -f "$(OPENOCD_PATH)/bin/wch-riscv.cfg"

OPENOCD_FLASH_COMMANDS = \
-c "program $< verify" -c wlink_reset_resume -c resume -c exit
```

> There are some problems on Linux on current stage (2023-03-02),
> you can read some details here: <https://www.wch.cn/bbs/thread-96333-1.html>

