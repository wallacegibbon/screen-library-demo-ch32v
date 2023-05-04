#include "core_systick.h"
#include "camera_ov2640.h"
#include "ch32v30x.h"
#include <stdint.h>

#define OV_RESET_SET (GPIOB->BSHR = GPIO_Pin_7)
#define OV_RESET_CLR (GPIOB->BCR = GPIO_Pin_7)
#define OV_PWDN_SET (GPIOC->BSHR = GPIO_Pin_7)
#define OV_PWDN_CLR (GPIOC->BCR = GPIO_Pin_7)

#define IIC_SCL_IN \
	do { GPIOB->CFGHR &= 0XFFFFF0FF; GPIOB->CFGHR |= 8 << 8; } while (0)

#define IIC_SCL_OUT \
	do { GPIOB->CFGHR &= 0XFFFFF0FF; GPIOB->CFGHR |= 3 << 8; } while (0)

#define IIC_SDA_IN \
	do { GPIOB->CFGHR &= 0XFFFF0FFF; GPIOB->CFGHR |= 8 << 12; } while (0)

#define IIC_SDA_OUT \
	do { GPIOB->CFGHR &= 0XFFFF0FFF; GPIOB->CFGHR |= 3 << 12; } while (0)

#define IIC_SDA_SET (GPIOB->BSHR = GPIO_Pin_11)
#define IIC_SDA_CLR (GPIOB->BCR = GPIO_Pin_11)
#define IIC_SCL_SET (GPIOB->BSHR = GPIO_Pin_10)
#define IIC_SCL_CLR (GPIOB->BCR = GPIO_Pin_10)

/// SDA In
#define SDA_IN_R (GPIOB->INDR & GPIO_Pin_11)

//#define DVP_Work_Mode 0
#define DVP_Work_Mode RGB565_MODE

/*
uint8_t JPEG_DVPDMAaddr0[OV2640_JPEG_WIDTH];
uint8_t JPEG_DVPDMAaddr1[OV2640_JPEG_WIDTH];
*/

uint8_t RGB565_DVPDMAaddr0[RGB565_COL_NUM * 2];
uint8_t RGB565_DVPDMAaddr1[RGB565_COL_NUM * 2];

int SCCB_write_reg(uint8_t reg_addr, uint8_t reg_data);
uint8_t SCCB_read_reg(uint8_t reg_addr);

/// Start Camera list of initialization configuration registers
static const uint8_t ov2640_init_reg_tbl[] = {
	/// select DSP register bank
	0xFF, 0x00,
	0x2C, 0xFF,
	0x2E, 0xDF,

	/// select sensor register bank
	0xFF, 0x01,
	0x3C, 0x32,

	/// internal frequency doublers OFF, no clock divider
	0x11, 0x00,

	/// 2x capability
	0x09, 0x02,

	/// enable horizontal mirror
	0x04, 0xD8,

	/// AGC control: auto; exposure control: auto
	0x13, 0xE5,

	/// AGC gain: 32x
	0x14, 0x48,

	0x2C, 0x0C,
	0x33, 0x78,
	0x3A, 0x33,
	0x3B, 0xFB,

	0x3E, 0x00,
	0x43, 0x11,
	0x16, 0x10,

	0x39, 0x92,

	0x35, 0xDA,
	0x22, 0x1A,
	0x37, 0xC3,
	0x23, 0x00,
	0x34, 0xC0,
	0x36, 0x1A,
	0x06, 0x88,
	0x07, 0xC0,
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

	/// line (? this comment comes from ChiTu demo project)
	0x46, 0x00,

	0x0C, 0x3C,

	0x61, 0x70,
	0x62, 0x80,
	0x7C, 0x05,

	0x20, 0x80,
	0x28, 0x30,
	0x6C, 0x00,
	0x6D, 0x80,
	0x6E, 0x00,
	0x70, 0x02,
	0x71, 0x94,
	0x73, 0xC1,
	0x3D, 0x34,
	0x5A, 0x57,

	/// UXGA (full size) mode
	0x12, 0x00,
	0x17, 0x11,
	0x18, 0x75,
	0x19, 0x01,
	0x1A, 0x97,

	/// PCLK frequency NO divide
	0x32, 0x36,

	0x03, 0x0F,
	0x37, 0x40,

	0x4F, 0xCA,
	0x50, 0xA8,
	0x5A, 0x23,
	0x6D, 0x00,
	0x6D, 0x38,

	/// select DSP register bank
	0xFF, 0x00,
	0xE5, 0x7F,
	0xF9, 0xC0,
	0x41, 0x24,

	/// reset
	0xE0, 0x14,

	0x76, 0xFF,
	0x33, 0xA0,
	0x42, 0x20,
	0x43, 0x18,
	0x4C, 0x00,
	0x87, 0xD5,
	0x88, 0x3F,
	0xD7, 0x03,
	0xD9, 0x10,
	0xD3, 0x82,

	0xC8, 0x08,
	0xC9, 0x80,

	0x7C, 0x00,
	0x7D, 0x00,
	0x7C, 0x03,
	0x7D, 0x48,
	0x7D, 0x48,
	0x7C, 0x08,
	0x7D, 0x20,
	0x7D, 0x10,
	0x7D, 0x0E,

	0x90, 0x00,
	0x91, 0x0E,
	0x91, 0x1A,
	0x91, 0x31,
	0x91, 0x5A,
	0x91, 0x69,
	0x91, 0x75,
	0x91, 0x7E,
	0x91, 0x88,
	0x91, 0x8F,
	0x91, 0x96,
	0x91, 0xA3,
	0x91, 0xAF,
	0x91, 0xC4,
	0x91, 0xD7,
	0x91, 0xE8,
	0x91, 0x20,

	0x92, 0x00,
	0x93, 0x06,
	0x93, 0xE3,
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

	0xA4, 0x00,
	0xA8, 0x00,
	0xC5, 0x11,
	0xC6, 0x51,
	0xBF, 0x80,
	0xC7, 0x10,
	0xB6, 0x66,
	0xB8, 0xA5,
	0xB7, 0x64,
	0xB9, 0x7C,
	0xB3, 0xAF,
	0xB4, 0x97,
	0xB5, 0xFF,
	0xB0, 0xC5,
	0xB1, 0x94,
	0xB2, 0x0F,
	0xC4, 0x5C,

	0xC0, 0xC8,
	0xC1, 0x96,
	0x8C, 0x00,
	0x86, 0x3D,
	0x50, 0x00,
	0x51, 0x50,
	0x52, 0x2C,
	0x53, 0x00,
	0x54, 0x00,
	0x55, 0x88,

	0x5A, 0x2C,
	0x5B, 0x2C,
	0x5C, 0x00,

	0xD3, 0x02,

	0xC3, 0xED,
	0x7F, 0x00,

	/// RGB565, low byte first
	0xDA, 0x09,

	0xE5, 0x1F,
	0xE1, 0x67,
	0xE0, 0x00,
	0xDD, 0x7F,

	/// do NOT bypass DSP. (disable sensor out directly)
	0x05, 0x00,
};

/// RGB565
static const uint8_t ov2640_rgb565_reg_tbl[] = {
	/// select DSP register bank
	0xFF, 0x00,
	/// register 0xDA controls IMAGE_MODE
	0xDA, 0x08,
	0xD7, 0x03,
	0xDF, 0x02,
	0x33, 0xA0,
	0x3C, 0x00,
	0xE1, 0x67,
	/// select sensor register bank
	0xFF, 0x01,
	0xE0, 0x00,
	0xE1, 0x00,
	0xE5, 0x00,
	0xD7, 0x00,
	0xDA, 0x00,
	0xE0, 0x00,
};

/// YUV422
static const uint8_t ov2640_yuv422_reg_tbl[] = {
	/// select DSP register bank
	0xFF, 0x00,
	/// register 0xDA controls IMAGE_MODE
	0xDA, 0x10,
	0xD7, 0x03,
	0xDF, 0x00,
	0x33, 0x80,
	0x3C, 0x40,
	0xE1, 0x77,
	0x00, 0x00,
};

/// JPEG
static const uint8_t ov2640_jpeg_reg_tbl[] = {
	/// select sensor register bank
	0xFF, 0x01,
	0xE0, 0x14,
	0xE1, 0x77,
	0xE5, 0x1F,
	0xD7, 0x03,
	0xDA, 0x10,
	0xE0, 0x00,
};

static void DVP_gpio_initialize() {
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

void SCCB_gpio_initialize() {
	IIC_SCL_OUT;
	IIC_SDA_OUT;
	IIC_SCL_SET;
	IIC_SDA_SET;
}

void DVP_initialize() {
	NVIC_InitTypeDef NVIC_InitStructure = {0};

	DVP_gpio_initialize();

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DVP, ENABLE);

	DVP->CR0 &= ~RB_DVP_MSK_DAT_MOD;

#if (DVP_Work_Mode == RGB565_MODE)
	/// VSYNC & HSYNC: High level active
	DVP->CR0 |= RB_DVP_D10_MOD | RB_DVP_V_POLAR;
	DVP->CR1 &= ~(RB_DVP_ALL_CLR | RB_DVP_RCV_CLR);
	DVP->ROW_NUM = RGB565_ROW_NUM;	// rows
	DVP->COL_NUM = RGB565_COL_NUM * 2; // cols

	DVP->DMA_BUF0 = (uint32_t) RGB565_DVPDMAaddr0; // DMA addr0
	DVP->DMA_BUF1 = (uint32_t) RGB565_DVPDMAaddr1; // DMA addr1
#endif

	/// Set frame capture rate
	DVP->CR1 &= ~RB_DVP_FCRC;
	DVP->CR1 |= DVP_RATE_100P; // 100%

	/// These 5 interrupt shares DVP_IRQHandler
	DVP->IER |= RB_DVP_IE_STP_FRM;
	DVP->IER |= RB_DVP_IE_FIFO_OV;
	DVP->IER |= RB_DVP_IE_FRM_DONE;
	DVP->IER |= RB_DVP_IE_ROW_DONE;
	DVP->IER |= RB_DVP_IE_STR_FRM;

	NVIC_InitStructure.NVIC_IRQChannel = DVP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DVP->CR1 |= RB_DVP_DMA_EN;
	DVP->CR0 |= RB_DVP_ENABLE;
}

void ov2640_send_command_table(const uint8_t *cmd_tbl, int size) {
	int i;
	for (i = 0; i < size / 2; i++)
		SCCB_write_reg(cmd_tbl[i * 2], cmd_tbl[i * 2 + 1]);
}

int ov2640_initialize() {
	int reg;

	SCCB_gpio_initialize();

	OV_PWDN_CLR;
	delay_ms(10);
	OV_RESET_CLR;
	delay_ms(10);
	OV_RESET_SET;

	/// select sensor register bank
	SCCB_write_reg(0xFF, 0x01);
	/// initiates system reset, all registers are set to default value.
	SCCB_write_reg(0x12, 0x80);
	delay_ms(50);

	/// Menufacturer ID MSB nad MLB
	reg = SCCB_read_reg(0x1C);
	reg <<= 8;
	reg |= SCCB_read_reg(0x1D);

	if (reg != OV2640_MID)
		return 1;

	/// Product ID MSB and MLB
	reg = SCCB_read_reg(0x0A);
	reg <<= 8;
	reg |= SCCB_read_reg(0x0B);

	if (reg != OV2640_PID)
		return 2;

	ov2640_send_command_table(
		ov2640_init_reg_tbl, sizeof(ov2640_init_reg_tbl)
	);

	return 0;
}

void ov2640_JPEG_mode() {
	ov2640_send_command_table(
		ov2640_yuv422_reg_tbl, sizeof(ov2640_yuv422_reg_tbl)
	);
	ov2640_send_command_table(
		ov2640_jpeg_reg_tbl, sizeof(ov2640_jpeg_reg_tbl)
	);
}

void ov2640_RGB565_mode() {
	ov2640_send_command_table(
		ov2640_rgb565_reg_tbl, sizeof(ov2640_rgb565_reg_tbl)
	);
}

/// Register 5A, 5B and 5C control the output size of OV2640.
/// 5A: OUTW[7:0]; 5B: OUTH[7:0]; 5C: ([2]: OUTH[8]; [1:0]: OUTW[9:8])
int ov2640_outsize_set(uint16_t image_width, uint16_t image_height) {
	uint16_t outsize_width, outsize_height;
	uint8_t tmp;

	if (image_width % 4 || image_height % 4)
		return 1;

	outsize_width = image_width / 4;
	outsize_height = image_height / 4;

	SCCB_write_reg(0xFF, 0);
	SCCB_write_reg(0xE0, 4);
	SCCB_write_reg(0x5A, outsize_width & 0xFF);
	SCCB_write_reg(0x5B, outsize_height & 0xFF);
	tmp = (outsize_width >> 8) & 0x03;
	tmp |= (outsize_height >> 6) & 0x04;
	SCCB_write_reg(0x5C, tmp);
	SCCB_write_reg(0xE0, 0);

	return 0;
}

int ov2640_speed_set(uint8_t pclk_div, uint8_t xclk_div) {
	SCCB_write_reg(0xFF, 0);
	SCCB_write_reg(0xD3, pclk_div);

	SCCB_write_reg(0xFF, 1);
	SCCB_write_reg(0x11, xclk_div);
}

void ov2640_JPEG_mode_initialize() {
	ov2640_JPEG_mode();
	ov2640_outsize_set(OV2640_JPEG_WIDTH, OV2640_JPEG_HEIGHT);
	ov2640_speed_set(30, 1);
}

void ov2640_RGB565_mode_initialize() {
	ov2640_RGB565_mode();
	ov2640_outsize_set(OV2640_RGB565_WIDTH, OV2640_RGB565_HEIGHT);
	ov2640_speed_set(15, 3);
}

void SCCB_start() {
	IIC_SDA_SET;
	IIC_SCL_SET;
	delay_us(50);
	IIC_SDA_CLR;
	delay_us(50);
	IIC_SCL_CLR;
}

void SCCB_stop() {
	IIC_SDA_CLR;
	delay_us(50);
	IIC_SCL_SET;
	delay_us(50);
	IIC_SDA_SET;
	delay_us(50);
}

void SCCB_no_ack() {
	delay_us(50);
	IIC_SDA_SET;
	IIC_SCL_SET;
	delay_us(50);
	IIC_SCL_CLR;
	delay_us(50);
	IIC_SDA_CLR;
	delay_us(50);
}

int SCCB_write_byte(uint8_t data) {
	int i, t;
	for (i = 0; i < 8; i++) {
		if (data & 0x80) IIC_SDA_SET;
		else IIC_SDA_CLR;

		data <<= 1;
		delay_us(50);
		IIC_SCL_SET;
		delay_us(50);
		IIC_SCL_CLR;
	}

	IIC_SDA_IN;
	delay_us(50);
	IIC_SCL_SET;
	delay_us(50);

	t = !!SDA_IN_R;

	IIC_SCL_CLR;
	IIC_SDA_OUT;

	return t;
}

uint8_t SCCB_read_byte() {
	int i, t = 0;

	IIC_SDA_IN;

	for (i = 0; i < 8; i++) {
		delay_us(50);
		IIC_SCL_SET;

		t <<= 1;
		if (SDA_IN_R) t |= 1;

		delay_us(50);
		IIC_SCL_CLR;
	}

	IIC_SDA_OUT;
	return t;
}

int SCCB_write_reg(uint8_t reg_addr, uint8_t reg_data) {
	int r = 0;
	SCCB_start();

	if (SCCB_write_byte(OV2640_SCCB_ID)) r = 1;
	delay_us(100);

	if (SCCB_write_byte(reg_addr)) r = 1;
	delay_us(100);

	if (SCCB_write_byte(reg_data)) r = 1;
	delay_us(100);

	SCCB_stop();
	return r;
}

uint8_t SCCB_read_reg(uint8_t reg_addr) {
	int r = 0;
	SCCB_start();
	SCCB_write_byte(OV2640_SCCB_ID);
	delay_us(100);
	SCCB_write_byte(reg_addr);
	delay_us(100);
	SCCB_stop();
	delay_us(100);

	SCCB_start();
	SCCB_write_byte(OV2640_SCCB_ID | 0x01);
	delay_us(100);
	r = SCCB_read_byte();
	SCCB_no_ack();
	SCCB_stop();

	return r;
}

