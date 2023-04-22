#include "ch32v30x.h"
#include <stdint.h>

/// Start Camera list of initialization configuration registers
static const uint8_t ov2640_init_reg_tbl[][2] = {
	0xFF, 0x00,
	0x2C, 0xFF,
	0x2E, 0xDF,
	0xFF, 0x01,
	0x3C, 0x32,

	0x11, 0x00,
	0x09, 0x02,
	0x04, 0xD8,
	0x13, 0xe5,
	0x14, 0x48,
	0x2C, 0x0C,
	0x33, 0x78,
	0x3A, 0x33,
	0x3B, 0xFB,

	0x3e, 0x00,
	0x43, 0x11,
	0x16, 0x10,

	0x39, 0x92,

	0x35, 0xda,
	0x22, 0x1a,
	0x37, 0xc3,
	0x23, 0x00,
	0x34, 0xc0,
	0x36, 0x1a,
	0x06, 0x88,
	0x07, 0xc0,
	0x0D, 0x87,
	0x0E, 0x41,
	0x4C, 0x00,

	0x48, 0x00,
	0x5B, 0x00,
	0x42, 0x03,

	0x4A, 0x81,
	0x21, 0x99,

	0x24, 0x40,
	0x25, 0x38,
	0x26, 0x82,
	0x5C, 0x00,
	0x63, 0x00,
	0x46, 0x00, // line
	0x0C, 0x3C,

	0x61, 0x70,
	0x62, 0x80,
	0x7c, 0x05,

	0x20, 0x80,
	0x28, 0x30,
	0x6c, 0x00,
	0x6d, 0x80,
	0x6e, 0x00,
	0x70, 0x02,
	0x71, 0x94,
	0x73, 0xc1,
	0x3d, 0x34,
	0x5A, 0x57,
	0x12, 0x00, // UXGA

	0x17, 0x11,
	0x18, 0x75, // UXGA
	0x19, 0x01, // UXGA
	0x1a, 0x97,
	0x32, 0x36,
	0x03, 0x0f,
	0x37, 0x40,

	0x4f, 0xca,
	0x50, 0xa8,
	0x5A, 0x23,
	0x6d, 0x00,
	0x6d, 0x38,

	0xFF, 0x00,
	0xe5, 0x7f,
	0xf9, 0xc0,
	0x41, 0x24,
	0xe0, 0x14,
	0x76, 0xFF,
	0x33, 0xa0,
	0x42, 0x20,
	0x43, 0x18,
	0x4C, 0x00,
	0x87, 0xd5,
	0x88, 0x3f,
	0xd7, 0x03,
	0xd9, 0x10,
	0xd3, 0x82,

	0xc8, 0x08,
	0xc9, 0x80,

	0x7c, 0x00,
	0x7d, 0x00,
	0x7c, 0x03,
	0x7d, 0x48,
	0x7d, 0x48,
	0x7c, 0x08,
	0x7d, 0x20,
	0x7d, 0x10,
	0x7d, 0x0E,

	0x90, 0x00,
	0x91, 0x0E,
	0x91, 0x1a,
	0x91, 0x31,
	0x91, 0x5A,
	0x91, 0x69,
	0x91, 0x75,
	0x91, 0x7e,
	0x91, 0x88,
	0x91, 0x8f,
	0x91, 0x96,
	0x91, 0xa3,
	0x91, 0xaf,
	0x91, 0xc4,
	0x91, 0xd7,
	0x91, 0xe8,
	0x91, 0x20,

	0x92, 0x00,
	0x93, 0x06,
	0x93, 0xe3,
	0x93, 0x05,
	0x93, 0x05,
	0x93, 0x00,
	0x93, 0x04,
	0x93, 0x00,
	0x93, 0x00,
	0x93, 0x00,
	0x93, 0x00,
	0x93, 0x00,
	0x93, 0x00,
	0x93, 0x00,

	0x96, 0x00,
	0x97, 0x08,
	0x97, 0x19,
	0x97, 0x02,
	0x97, 0x0C,
	0x97, 0x24,
	0x97, 0x30,
	0x97, 0x28,
	0x97, 0x26,
	0x97, 0x02,
	0x97, 0x98,
	0x97, 0x80,
	0x97, 0x00,
	0x97, 0x00,

	0xC3, 0xEF,

	0xa4, 0x00,
	0xa8, 0x00,
	0xc5, 0x11,
	0xc6, 0x51,
	0xbf, 0x80,
	0xc7, 0x10,
	0xb6, 0x66,
	0xb8, 0xA5,
	0xb7, 0x64,
	0xb9, 0x7C,
	0xb3, 0xaf,
	0xb4, 0x97,
	0xb5, 0xFF,
	0xb0, 0xC5,
	0xb1, 0x94,
	0xb2, 0x0f,
	0xc4, 0x5C,

	0xc0, 0xc8,
	0xc1, 0x96,
	0x8c, 0x00,
	0x86, 0x3d,
	0x50, 0x00,
	0x51, 0x50,
	0x52, 0x2C,
	0x53, 0x00,
	0x54, 0x00,
	0x55, 0x88,

	0x5A, 0x2C,
	0x5B, 0x2C,
	0x5C, 0x00,

	0xd3, 0x02,

	0xc3, 0xed,
	0x7f, 0x00,

	0xda, 0x09,

	0xe5, 0x1f,
	0xe1, 0x67,
	0xe0, 0x00,
	0xdd, 0x7f,
	0x05, 0x00,

	// 0xFF, 0x00,
	// 0x2C, 0xFF,
	// 0x2E, 0xDF,
	// 0xFF, 0x01,
	// 0x3C, 0x32,
	//
	// 0x11, 0x00,
	// 0x09, 0x02,
	// 0x04, 0xD8, // mirror & revert
	// 0x13, 0xe5,
	// 0x14, 0x48,
	// 0x2C, 0x0C,
	// 0x33, 0x78,
	// 0x3A, 0x33,
	// 0x3B, 0xFB,
	//
	// 0x3e, 0x00,
	// 0x43, 0x11,
	// 0x16, 0x10,
	//
	// 0x39, 0x92,
	//
	// 0x35, 0xda,
	// 0x22, 0x1a,
	// 0x37, 0xc3,
	// 0x23, 0x00,
	// 0x34, 0xc0,
	// 0x36, 0x1a,
	// 0x06, 0x88,
	// 0x07, 0xc0,
	// 0x0D, 0x87,
	// 0x0E, 0x41,
	// 0x4C, 0x00,
	// 0x48, 0x00,
	// 0x5B, 0x00,
	// 0x42, 0x03,
	//
	// 0x4A, 0x81,
	// 0x21, 0x99,
	//
	// 0x24, 0x40,
	// 0x25, 0x38,
	// 0x26, 0x82,
	// 0x5C, 0x00,
	// 0x63, 0x00,
	// 0x46, 0x22,
	// 0x0C, 0x3C,
	//
	// 0x61, 0x70,
	// 0x62, 0x80,
	// 0x7c, 0x05,
	//
	// 0x20, 0x80,
	// 0x28, 0x30,
	// 0x6c, 0x00,
	// 0x6d, 0x80,
	// 0x6e, 0x00,
	// 0x70, 0x02,
	// 0x71, 0x94,
	// 0x73, 0xc1,
	//
	// 0x3d, 0x34,
	// 0x5A, 0x57,
	//
	// 0x12, 0x40, // SVGA 800*600
	// 0x17, 0x11,
	// 0x18, 0x43,
	// 0x19, 0x00,
	// 0x1a, 0x4b,
	// 0x32, 0x09,
	// 0x37, 0xc0,
	//
	// 0x4f, 0xca,
	// 0x50, 0xa8,
	// 0x5A, 0x23,
	// 0x6d, 0x00,
	// 0x3d, 0x38,
	//
	// 0xFF, 0x00,
	// 0xe5, 0x7f,
	// 0xf9, 0xc0,
	// 0x41, 0x24,
	// 0xe0, 0x14,
	// 0x76, 0xFF,
	// 0x33, 0xa0,
	// 0x42, 0x20,
	// 0x43, 0x18,
	// 0x4C, 0x00,
	// 0x87, 0xd5,
	// 0x88, 0x3f,
	// 0xd7, 0x03,
	// 0xd9, 0x10,
	// 0xd3, 0x82,
	//
	// 0xc8, 0x08,
	// 0xc9, 0x80,
	//
	// 0x7c, 0x00,
	// 0x7d, 0x00,
	// 0x7c, 0x03,
	// 0x7d, 0x48,
	// 0x7d, 0x48,
	// 0x7c, 0x08,
	// 0x7d, 0x20,
	// 0x7d, 0x10,
	// 0x7d, 0x0E,
	//
	// 0x90, 0x00,
	// 0x91, 0x0E,
	// 0x91, 0x1a,
	// 0x91, 0x31,
	// 0x91, 0x5A,
	// 0x91, 0x69,
	// 0x91, 0x75,
	// 0x91, 0x7e,
	// 0x91, 0x88,
	// 0x91, 0x8f,
	// 0x91, 0x96,
	// 0x91, 0xa3,
	// 0x91, 0xaf,
	// 0x91, 0xc4,
	// 0x91, 0xd7,
	// 0x91, 0xe8,
	// 0x91, 0x20,
	//
	// 0x92, 0x00,
	// 0x93, 0x06,
	// 0x93, 0xe3,
	// 0x93, 0x05,
	// 0x93, 0x05,
	// 0x93, 0x00,
	// 0x93, 0x04,
	// 0x93, 0x00,
	// 0x93, 0x00,
	// 0x93, 0x00,
	// 0x93, 0x00,
	// 0x93, 0x00,
	// 0x93, 0x00,
	// 0x93, 0x00,
	//
	// 0x96, 0x00,
	// 0x97, 0x08,
	// 0x97, 0x19,
	// 0x97, 0x02,
	// 0x97, 0x0C,
	// 0x97, 0x24,
	// 0x97, 0x30,
	// 0x97, 0x28,
	// 0x97, 0x26,
	// 0x97, 0x02,
	// 0x97, 0x98,
	// 0x97, 0x80,
	// 0x97, 0x00,
	// 0x97, 0x00,
	//
	// 0xc3, 0xed,
	// 0xa4, 0x00,
	// 0xa8, 0x00,
	// 0xc5, 0x11,
	// 0xc6, 0x51,
	// 0xbf, 0x80,
	// 0xc7, 0x10,
	// 0xb6, 0x66,
	// 0xb8, 0xA5,
	// 0xb7, 0x64,
	// 0xb9, 0x7C,
	// 0xb3, 0xaf,
	// 0xb4, 0x97,
	// 0xb5, 0xFF,
	// 0xb0, 0xC5,
	// 0xb1, 0x94,
	// 0xb2, 0x0f,
	// 0xc4, 0x5C,
	//
	// 0xc0, 0x64,
	// 0xc1, 0x4B,
	// 0x8c, 0x00,
	// 0x86, 0x3D,
	// 0x50, 0x00,
	// 0x51, 0xC8,
	// 0x52, 0x96,
	// 0x53, 0x00,
	// 0x54, 0x00,
	// 0x55, 0x00,
	// 0x5A, 0x50,
	// 0x5B, 0x3C,
	// 0x5C, 0x00,
	//
	// 0xd3, 0x82, // auto (be careful)
	//
	// 0xc3, 0xed,
	// 0x7f, 0x00,
	//
	// 0xda, 0x09,
	//
	// 0xe5, 0x1f,
	// 0xe1, 0x67,
	// 0xe0, 0x00,
	// 0xdd, 0x7f,
	// 0x05, 0x00,
};

/// YUV422
static const uint8_t ov2640_yuv422_reg_tbl[][2] = {
	0xFF, 0x00,
	0xDA, 0x10,
	0xD7, 0x03,
	0xDF, 0x00,
	0x33, 0x80,
	0x3C, 0x40,
	0xE1, 0x77,
	0x00, 0x00,
};

/// JPEG
static const uint8_t ov2640_jpeg_reg_tbl[][2] = {
	0xFF, 0x01,
	0xE0, 0x14,
	0xE1, 0x77,
	0xE5, 0x1F,
	0xD7, 0x03,
	0xDA, 0x10,
	0xE0, 0x00,
};

/// RGB565
static const uint8_t ov2640_rgb565_reg_tbl[][2] = {
	0xFF, 0x00,
	0xDA, 0x08,
	0xD7, 0x03,
	0xDF, 0x02,
	0x33, 0xA0,
	0x3C, 0x00,
	0xE1, 0x67,
	0xFF, 0x01,
	0xE0, 0x00,
	0xE1, 0x00,
	0xE5, 0x00,
	0xD7, 0x00,
	0xDA, 0x00,
	0xE0, 0x00,
};

static void dvp_gpio_initialize() {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

int ov2640_initialize() {
	dvp_gpio_initialize();

	return 0;
}

