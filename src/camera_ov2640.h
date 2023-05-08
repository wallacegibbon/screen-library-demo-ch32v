#ifndef __CAMERA_OV2640_H
#define __CAMERA_OV2640_H

#include <stdint.h>

extern uint8_t RGB565_DVPDMAaddr0[];
extern uint8_t RGB565_DVPDMAaddr1[];

int ov2640_initialize();
void ov2640_RGB565_mode_initialize();
void ov2640_JPEG_mode_initialize();
void DVP_initialize();

#endif

