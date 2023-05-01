#ifndef __CAMERA_OV2640_H
#define __CAMERA_OV2640_H

#define JPEG_DVPDMAaddr0 0x20005000
#define JPEG_DVPDMAaddr1 (0x20005000 + OV2640_JPEG_WIDTH)

#define RGB565_DVPDMAaddr0 0x2000A000
#define RGB565_DVPDMAaddr1 (0x2000A000 + RGB565_COL_NUM * 2)

int ov2640_initialize();
void ov2640_RGB565_mode_initialize();
void ov2640_JPEG_mode_initialize();
void DVP_initialize();

#endif

