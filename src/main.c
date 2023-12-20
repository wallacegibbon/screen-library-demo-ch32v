#include "sc_ssd1306.h"
#include "sc_ssd1306_ch32v_i2c.h"
#include "sc_st7789.h"
#include "sc_st7789_ch32v_fsmc.h"
//#include "sc_st7735_ch32v_spi.h"
#include "camera_ov2640.h"
#include "ch32v30x.h"
#include "ch32v_debug.h"
#include "core_systick.h"
#include "sc_color.h"
#include "sc_common.h"
#include "sc_painter.h"
#include "sc_st7735.h"
#include <math.h>
#include <stdio.h>

/// global variables that should be initialzed to the size of the target screen on startup. (before DVP initializing and DVP interrupts)
uint16_t camera_screen_width;
uint16_t camera_screen_height;

void fancy_display(struct painter *painter) {
	static int current_cnt = 0, step = 1;
	struct point p;
	struct point size;
	uint32_t color;
	int i;

	painter_size(painter, &size);
	point_initialize(&p, size.x / 2, size.y / 2);
	for (i = 0; i < 31; i++) {
		color = (ABS(current_cnt - i) < 3) ? BLACK_24bit : CYAN_24bit;
		painter_draw_circle(painter, p, i, color);
	}
	painter_flush(painter);

	if (current_cnt == 31)
		step = -1;
	else if (current_cnt == 0)
		step = 1;

	current_cnt += step;
}

void initialize_screen_1(struct ssd1306_screen *screen, struct ssd1306_adaptor_ch32v_i2c *adaptor) {
	ssd1306_adaptor_ch32v_i2c_initialize(adaptor, 0x3C);
	ssd1306_initialize(screen, (const struct ssd1306_adaptor_i **)adaptor);
	ssd1306_display_on(screen);
}

/*
void initialize_screen_2(struct st7735_screen *screen, struct st7735_adaptor_ch32v_spi *adaptor) {
	st7735_adaptor_ch32v_spi_initialize(adaptor, ...);
	st7735_initialize(screen, (const struct st7735_adaptor_i **)adaptor);
}
*/

void lcd_bg_pwm_initialize(uint16_t prescale, uint16_t period, uint16_t compare_value) {
	TIM_TimeBaseInitTypeDef tim_base_init;
	TIM_OCInitTypeDef tim_oc_init;
	GPIO_InitTypeDef gpio_init = {0};

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_TIM1, ENABLE);

	/// BG LED
	gpio_init.GPIO_Pin = GPIO_Pin_14;
	gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio_init);

	/// TIMx->ATRLR
	tim_base_init.TIM_Period = period;

	/// TIMx->PSC
	tim_base_init.TIM_Prescaler = prescale;

	tim_base_init.TIM_ClockDivision = TIM_CKD_DIV1;
	tim_base_init.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM1, &tim_base_init);

	/// `TIM1_CH2N` is mapped to `PB14` (which is connected to LEDK)
	/// PB14 = 1 (CH2N = 1 & CH2 = 0) will open background LED.
	tim_oc_init.TIM_OCMode = TIM_OCMode_PWM2;
	tim_oc_init.TIM_OutputState = TIM_OutputState_Disable;
	tim_oc_init.TIM_OutputNState = TIM_OutputNState_Enable;

	/// TIMx->CH2CVR
	tim_oc_init.TIM_Pulse = compare_value;

	tim_oc_init.TIM_OCNPolarity = TIM_OCNPolarity_High;
	TIM_OC2Init(TIM1, &tim_oc_init);

	TIM_CtrlPWMOutputs(TIM1, ENABLE);
	TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Disable);
	TIM_ARRPreloadConfig(TIM1, ENABLE);
	TIM_Cmd(TIM1, ENABLE);
}

void lcd_bg_set_brightness(uint16_t brightness) {
	TIM1->CH2CVR = brightness;
}

void initialize_screen_3(struct st7789_screen *screen, struct st7789_adaptor_ch32v_fsmc *adaptor) {
	st7789_adaptor_ch32v_fsmc_initialize(adaptor);

	st7789_initialize(screen, (const struct st7789_adaptor_i **)adaptor);

	// GPIO_SetBits(GPIOB, GPIO_Pin_14);

	lcd_bg_pwm_initialize(144 - 1, 100, 50);
	lcd_bg_set_brightness(20);
}

void graphic_play(struct painter *painter) {
	struct point p1, p2, size;
	struct text_painter text_painter;

	painter_clear(painter, BLACK_24bit);

	/// The default method do not flush, but overridden `clear` can do flush automatically.
	// painter_flush(painter);

	/// text drawing
	text_painter_initialize(&text_painter, painter);

	color_pair_initialize(&text_painter.color, RED_24bit, BLACK_24bit);
	point_initialize(&text_painter.pos, 0, 0);

	text_draw_string(&text_painter, "1.5 Programming!", 32);

	color_pair_initialize(&text_painter.color, GREEN_24bit, BLACK_24bit);
	point_initialize(&text_painter.pos, 0, 32);

	text_draw_string(&text_painter, "1.5 Programming!", 16);

	painter_size(painter, &size);

	point_initialize(&p1, size.x / 2 - 50, size.y / 2 - 20);
	point_initialize(&p2, size.x / 2 + 50, size.y / 2 + 20);
	painter_draw_rectangle(painter, p1, p2, YELLOW_24bit);

	point_initialize(&p1, size.x / 2 - 50, size.y / 2 - 20);
	painter_draw_circle(painter, p1, 5, MAGENTA_24bit);

	point_initialize(&p1, 10, size.y / 2 - 20);
	point_initialize(&p2, 10, size.y / 2 + 20);
	painter_draw_line(painter, p1, p2, WHITE_24bit);

	painter_flush(painter);

	while (1)
		fancy_display(painter);
}

void dma_lcd_initialize(uintptr_t periph_address) {
	DMA_InitTypeDef dma_init;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
	DMA_DeInit(DMA2_Channel5);

	dma_init.DMA_PeripheralBaseAddr = periph_address;
	dma_init.DMA_MemoryBaseAddr = ST7789_LCD_DATA;
	dma_init.DMA_DIR = DMA_DIR_PeripheralSRC;
	dma_init.DMA_BufferSize = 0;
	dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
	dma_init.DMA_MemoryInc = DMA_MemoryInc_Disable;
	dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dma_init.DMA_Mode = DMA_Mode_Normal;
	dma_init.DMA_Priority = DMA_Priority_Medium;
	dma_init.DMA_M2M = DMA_M2M_Enable;

	DMA_Init(DMA2_Channel5, &dma_init);
}

void camera_display(struct painter *painter, struct st7789_screen *screen) {
	struct point p1, p2, size;

	painter_size(painter, &size);
	/// initialize global variables that represent the size of the screen.
	camera_screen_width = size.x;
	camera_screen_height = size.y;

	painter_clear(painter, BLACK_24bit);
	// painter_clear(painter, RED_24bit);

	// point_initialize(&p1, 0, 0);
	// point_initialize(&p2, camera_screen_width - 1, camera_screen_height - 1);

	/// Make sure that `p2.y - p1.y == camera_screen_height - 1`, and `p1.y < 0`.
	point_initialize(&p1, 0, -1);
	point_initialize(&p2, camera_screen_width - 1, camera_screen_height - 1 - 1);
	// point_initialize(&p1, 0, -10);
	// point_initialize(&p2, camera_screen_width - 1, camera_screen_height - 1 - 10);
	st7789_set_address(screen, p1, p2);

	while (ov2640_initialize())
		delay_ms(500);

	delay_ms(100);
	ov2640_rgb565_mode_initialize();
	delay_ms(100);

	dma_lcd_initialize((uintptr_t)rgb565_dvp_dma_buffer0);
	dvp_initialize();

	while (1)
		;
}

void compass_display(struct painter *painter) {
	struct point p1, p2, size, center;
	int r = 50;
	float theta = 45.0 / 180.0 * M_PI;

	painter_clear(painter, BLACK_24bit);

	painter_size(painter, &size);
	center.x = size.x / 2;
	center.y = size.y / 2;

	point_initialize(&p1, center.x + r * cos(theta), center.y - r * sin(theta));
	painter_draw_line(painter, p1, center, RED_24bit);
	point_initialize(&p1, center.x - r * cos(theta), center.y + r * sin(theta));
	painter_draw_line(painter, p1, center, BLUE_24bit);
}

int main() {
	struct ssd1306_adaptor_ch32v_i2c adaptor1;
	struct ssd1306_screen screen1;
	// struct st7735_adaptor_ch32v_spi adaptor2;
	// struct st7735_screen screen2;
	struct st7789_adaptor_ch32v_fsmc adaptor3;
	struct st7789_screen screen3;
	struct painter painter;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	initialize_systick_interrupt();

	// initialize_screen_1(&screen1, &adaptor1);
	// initialize_screen_2(&screen2, &adaptor2);
	initialize_screen_3(&screen3, &adaptor3);

	// SSD1306_Screen_set_up_down_invert(&screen1);

	// painter.drawing_board = (const struct drawing_i **)&screen1;
	// painter.drawing_board = (const struct drawing_i **)&screen2;
	painter.drawing_board = (const struct drawing_i **)&screen3;

	usart_printf_initialize(115200);
	printf("System is ready now. SystemClk: %d\r\n", SystemCoreClock);

	// camera_display(&painter, &screen3);
	graphic_play(&painter);
	// compass_display(&painter);

	return 0;
}
