## Introduction

This is a demo for my [screen library](https://github.com/wallacegibbon/screen-library-mcu) for micro-controllers.


## Notes

Currently, This [customized OpenOCD](https://github.com/karlp/openocd-hacks) is being used.

If you use the OpenOCD in MounRiver:

```makefile
OPENOCD_PATH = $(HOME)/MRS_Toolchain_Linux_x64_V1.60/OpenOCD
OPENOCD = "$(OPENOCD_PATH)/bin/openocd"
OPENOCD_ARGS = -f "$(OPENOCD_PATH)/bin/wch-riscv.cfg"

OPENOCD_FLASH_COMMANDS = \
-c "program $< verify" -c wlink_reset_resume -c exit
```

> If you use the OpenOCD of some old-version MounRiver, you may face
> some problems: <https://www.wch.cn/bbs/thread-96333-1.html#4>


## Miscellaneous

Generating `compile_commands.json` for clangd:

```sh
bear -- make
```

When clangd does not work, you can set environment variable to see detailed log:

```sh
CLANGD_TRACE=/tmp/clangd_trace.json vi src/main.c
```

