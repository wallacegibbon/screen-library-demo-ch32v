#ifndef __CAMERA_OV2640_H
#define __CAMERA_OV2640_H

#include <stdint.h>

extern uint8_t *rgb565_dvp_dma_buffer0;
extern uint8_t *rgb565_dvp_dma_buffer1;

int ov2640_init();

void ov2640_rgb565_mode_init();

void dvp_init();

#endif
