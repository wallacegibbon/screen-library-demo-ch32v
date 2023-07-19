#ifndef __CAMERA_OV2640_H
#define __CAMERA_OV2640_H

#include <stdint.h>

extern uint8_t *RGB565_dvp_dma_buffer0;
extern uint8_t *RGB565_dvp_dma_buffer1;

int ov2640_initialize();
void ov2640_rgb565_mode_initialize();
void ov2640_JPEG_mode_initialize();
void dvp_initialize();

#endif

