#ifndef __CAMERA_OV2640_H
#define __CAMERA_OV2640_H

#include <stdint.h>

/// RGB565 PIXEL 320 * 240
#define RGB565_ROW_NUM 240
#define RGB565_COL_NUM 480 // Col * 2

#define OV2640_RGB565_HEIGHT 320
#define OV2640_RGB565_WIDTH 240

/// JPEG PIXEL 1024 * 768
#define OV2640_JPEG_HEIGHT 768
#define OV2640_JPEG_WIDTH 1024

#define OV2640_SCCB_ID 0X60

#define OV2640_MID 0X7FA2
#define OV2640_PID 0X2642

/*
#define JPEG_DVPDMAaddr0 ((uintptr_t) 0x20005000)
#define JPEG_DVPDMAaddr1 ((uintptr_t) 0x20005000 + OV2640_JPEG_WIDTH)

#define RGB565_DVPDMAaddr0 ((uintptr_t) 0x2000A000)
#define RGB565_DVPDMAaddr1 ((uintptr_t) 0x2000A000 + RGB565_COL_NUM * 2)
*/

/*
extern uint8_t JPEG_DVPDMAaddr0[OV2640_JPEG_WIDTH];
extern uint8_t JPEG_DVPDMAaddr1[OV2640_JPEG_WIDTH];
*/

extern uint8_t RGB565_DVPDMAaddr0[RGB565_COL_NUM * 2];
extern uint8_t RGB565_DVPDMAaddr1[RGB565_COL_NUM * 2];

int ov2640_initialize();
void ov2640_RGB565_mode_initialize();
void ov2640_JPEG_mode_initialize();
void DVP_initialize();

#endif

