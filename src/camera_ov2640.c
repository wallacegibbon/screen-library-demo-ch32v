#include "camera_ov2640.h"
#include "ch32v30x.h"
#include "core_systick.h"
#include <assert.h>
#include <stdint.h>
#include <unistd.h>

extern uint16_t camera_screen_width;
extern uint16_t camera_screen_height;

/// The size of buffer should be able to contains one ROW of data from DVP.
/// 	(since DVP will switch buffer when one ROW of data is finished.)
///
/// In RGB565 mode, every pixel takes 2 bytes. But the DVP of MCU works in
/// 10-bit mode, every 10-bit data takes 2 bytes, so every pixel takes 4 bytes.
uint8_t *rgb565_dvp_dma_buffer0;
uint8_t *rgb565_dvp_dma_buffer1;

int sccb_write_reg(uint8_t reg_addr, uint8_t reg_data);
uint8_t sccb_read_reg(uint8_t reg_addr);

#define RGB565_COL_NUM 320
#define RGB565_ROW_NUM 240

#define OV2640_MID 0X7FA2
#define OV2640_PID 0X2642
#define OV2640_SCCB_ID 0X60

// clang-format off
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

	/// upside down
	//0x04, 0xD8,

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
// clang-format on

static void dvp_gpio_init() {
	GPIO_InitTypeDef gpio_init;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);

	gpio_init.GPIO_Pin = GPIO_Pin_7;
	gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio_init);

	gpio_init.GPIO_Pin = GPIO_Pin_7;
	gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &gpio_init);

	gpio_init.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_9 | GPIO_Pin_10;
	gpio_init.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &gpio_init);

	gpio_init.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
	gpio_init.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &gpio_init);

	gpio_init.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
	gpio_init.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC, &gpio_init);
}

static inline void ov_reset_set() { GPIOB->BSHR = GPIO_Pin_7; }
static inline void ov_reset_clr() { GPIOB->BCR = GPIO_Pin_7; }
static inline void ov_pwdn_set() { GPIOC->BSHR = GPIO_Pin_7; }
static inline void ov_pwdn_clr() { GPIOC->BCR = GPIO_Pin_7; }

static inline void iic_sda_set() { GPIOB->BSHR = GPIO_Pin_11; }
static inline void iic_sda_clr() { GPIOB->BCR = GPIO_Pin_11; }
static inline void iic_scl_set() { GPIOB->BSHR = GPIO_Pin_10; }
static inline void iic_scl_clr() { GPIOB->BCR = GPIO_Pin_10; }

static inline int iic_sda_read() {
	return GPIOB->INDR & GPIO_Pin_11;
}

static inline void iic_scl_in() {
	GPIOB->CFGHR &= 0XFFFFF0FF;
	GPIOB->CFGHR |= 8 << 8;
}

static inline void iic_scl_out() {
	GPIOB->CFGHR &= 0XFFFFF0FF;
	GPIOB->CFGHR |= 3 << 8;
}

static inline void iic_sda_in() {
	GPIOB->CFGHR &= 0XFFFF0FFF;
	GPIOB->CFGHR |= 8 << 12;
}

static inline void iic_sda_out() {
	GPIOB->CFGHR &= 0XFFFF0FFF;
	GPIOB->CFGHR |= 3 << 12;
}

void sccb_gpio_init() {
	iic_scl_out();
	iic_sda_out();
	iic_scl_set();
	iic_sda_set();
}

/// This function should be called after `camera_screen_width` got initialized.
void dvp_init() {
	NVIC_InitTypeDef nvic_init = {0};

	// assert(camera_screen_width > 0);
	// assert(camera_screen_height > 0);
	// assert(camera_screen_width <= RGB565_COL_NUM);
	// assert(camera_screen_height <= RGB565_ROW_NUM);

	dvp_gpio_init();

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DVP, ENABLE);

	DVP->CR0 &= ~RB_DVP_MSK_DAT_MOD;

	/// VSYNC & HSYNC: High level active
	DVP->CR0 |= RB_DVP_D10_MOD | RB_DVP_V_POLAR;
	DVP->CR1 &= ~(RB_DVP_ALL_CLR | RB_DVP_RCV_CLR);

	/// COL_NUM represents the PCLK number of one row of data.
	DVP->COL_NUM = RGB565_COL_NUM * 4;
	/// ROW_NUM does't matter in this program since we don't use FRM_DONE.
	DVP->ROW_NUM = RGB565_ROW_NUM;

	/// DVP will be cropped to fit into the screen.
	rgb565_dvp_dma_buffer0 = sbrk(camera_screen_width * 4);
	rgb565_dvp_dma_buffer1 = sbrk(camera_screen_width * 4);
	// assert(RGB565_dvp_dma_buffer0 > 0);
	// assert(RGB565_dvp_dma_buffer1 > 0);

	DVP->DMA_BUF0 = (uintptr_t)rgb565_dvp_dma_buffer0;
	DVP->DMA_BUF1 = (uintptr_t)rgb565_dvp_dma_buffer1;

	/// When Crop is enabled, COL_NUM and ROW_NUM will take no effect,
	/// and CAPCNT and VLINE define the size.

	/// start x position
	DVP->HOFFCNT = (RGB565_COL_NUM - camera_screen_width) * 2 / 2;
	/// start y position
	DVP->VST = (RGB565_ROW_NUM - camera_screen_height) * 2 / 2;
	/// size of the crop window
	DVP->CAPCNT = camera_screen_width * 2;
	DVP->VLINE = camera_screen_height * 2;

	DVP->CR1 |= RB_DVP_CROP;

	/// Set frame capture rate
	DVP->CR1 &= ~RB_DVP_FCRC;
	DVP->CR1 |= DVP_RATE_100P;

	/// These 5 interrupts share DVP_IRQHandler
	DVP->IER |= RB_DVP_IE_STP_FRM;
	DVP->IER |= RB_DVP_IE_FIFO_OV;
	DVP->IER |= RB_DVP_IE_FRM_DONE;
	DVP->IER |= RB_DVP_IE_ROW_DONE;
	DVP->IER |= RB_DVP_IE_STR_FRM;

	nvic_init.NVIC_IRQChannel = DVP_IRQn;
	nvic_init.NVIC_IRQChannelPreemptionPriority = 1;
	nvic_init.NVIC_IRQChannelSubPriority = 0;
	nvic_init.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init);

	DVP->CR1 |= RB_DVP_DMA_EN;
	DVP->CR0 |= RB_DVP_ENABLE;
}

void ov2640_send_command_table(const uint8_t *cmd_tbl, int size) {
	int i;
	for (i = 0; i < size / 2; i++)
		sccb_write_reg(cmd_tbl[i * 2], cmd_tbl[i * 2 + 1]);
}

int ov2640_init() {
	int reg;

	sccb_gpio_init();

	ov_pwdn_clr();
	delay_ms(10);
	ov_reset_clr();
	delay_ms(10);
	ov_reset_set();

	/// select sensor register bank
	sccb_write_reg(0xFF, 0x01);
	/// initiates system reset, all registers are set to default value.
	sccb_write_reg(0x12, 0x80);
	delay_ms(50);

	/// Menufacturer ID MSB nad MLB
	reg = sccb_read_reg(0x1C);
	reg <<= 8;
	reg |= sccb_read_reg(0x1D);

	if (reg != OV2640_MID)
		return 1;

	/// Product ID MSB and MLB
	reg = sccb_read_reg(0x0A);
	reg <<= 8;
	reg |= sccb_read_reg(0x0B);

	if (reg != OV2640_PID)
		return 2;

	ov2640_send_command_table(ov2640_init_reg_tbl, sizeof(ov2640_init_reg_tbl));

	return 0;
}

void ov2640_jpeg_mode() {
	ov2640_send_command_table(ov2640_yuv422_reg_tbl, sizeof(ov2640_yuv422_reg_tbl));
	ov2640_send_command_table(ov2640_jpeg_reg_tbl, sizeof(ov2640_jpeg_reg_tbl));
}

void ov2640_rgb565_mode() {
	ov2640_send_command_table(ov2640_rgb565_reg_tbl, sizeof(ov2640_rgb565_reg_tbl));
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

	sccb_write_reg(0xFF, 0);
	sccb_write_reg(0xE0, 4);
	sccb_write_reg(0x5A, outsize_width & 0xFF);
	sccb_write_reg(0x5B, outsize_height & 0xFF);
	tmp = (outsize_width >> 8) & 0x03;
	tmp |= (outsize_height >> 6) & 0x04;
	sccb_write_reg(0x5C, tmp);
	sccb_write_reg(0xE0, 0);

	return 0;
}

void ov2640_speed_set(uint8_t pclk_div, uint8_t xclk_div) {
	sccb_write_reg(0xFF, 0);
	sccb_write_reg(0xD3, pclk_div);

	sccb_write_reg(0xFF, 1);
	sccb_write_reg(0x11, xclk_div);
}

void ov2640_rgb565_mode_init() {
	ov2640_rgb565_mode();
	ov2640_outsize_set(RGB565_COL_NUM, RGB565_ROW_NUM);
	// ov2640_speed_set(15, 3);
	ov2640_speed_set(1, 1);
}

void sccb_start() {
	iic_sda_set();
	iic_scl_set();
	delay_us(50);
	iic_sda_clr();
	delay_us(50);
	iic_scl_clr();
}

void sccb_stop() {
	iic_sda_clr();
	delay_us(50);
	iic_scl_set();
	delay_us(50);
	iic_sda_set();
	delay_us(50);
}

void sccb_no_ack() {
	delay_us(50);
	iic_sda_set();
	iic_scl_set();
	delay_us(50);
	iic_scl_clr();
	delay_us(50);
	iic_sda_clr();
	delay_us(50);
}

int sccb_write_byte(uint8_t data) {
	int i, t;
	for (i = 0; i < 8; i++) {
		if (data & 0x80)
			iic_sda_set();
		else
			iic_sda_clr();

		data <<= 1;
		delay_us(50);
		iic_scl_set();
		delay_us(50);
		iic_scl_clr();
	}

	iic_sda_in();
	delay_us(50);
	iic_scl_set();
	delay_us(50);

	t = !!iic_sda_read();

	iic_scl_clr();
	iic_sda_out();

	return t;
}

uint8_t sccb_read_byte() {
	int i, t = 0;

	iic_sda_in();

	for (i = 0; i < 8; i++) {
		delay_us(50);
		iic_scl_set();

		t <<= 1;
		if (iic_sda_read())
			t |= 1;

		delay_us(50);
		iic_scl_clr();
	}

	iic_sda_out();
	return t;
}

int sccb_write_reg(uint8_t reg_addr, uint8_t reg_data) {
	int r = 0;
	sccb_start();

	if (sccb_write_byte(OV2640_SCCB_ID))
		r = 1;
	delay_us(100);

	if (sccb_write_byte(reg_addr))
		r = 1;
	delay_us(100);

	if (sccb_write_byte(reg_data))
		r = 1;
	delay_us(100);

	sccb_stop();
	return r;
}

uint8_t sccb_read_reg(uint8_t reg_addr) {
	int r = 0;
	sccb_start();
	sccb_write_byte(OV2640_SCCB_ID);
	delay_us(100);
	sccb_write_byte(reg_addr);
	delay_us(100);
	sccb_stop();
	delay_us(100);

	sccb_start();
	sccb_write_byte(OV2640_SCCB_ID | 0x01);
	delay_us(100);
	r = sccb_read_byte();
	sccb_no_ack();
	sccb_stop();

	return r;
}
