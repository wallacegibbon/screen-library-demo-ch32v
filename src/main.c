#include "sc_ssd1306.h"
#include "sc_ssd1306_ch32v_i2c.h"
#include "sc_st7789.h"
#include "sc_st7789_ch32v_fsmc.h"
#include "sc_st7789_ch32v_hwspi.h"
// #include "sc_st7735_ch32v_spi.h"
#include "camera_ov2640.h"
#include "ch32v30x.h"
#include "ch32v_debug.h"
#include "core_systick.h"
#include "sc_color.h"
#include "sc_common.h"
#include "sc_painter.h"
#include "sc_st7735.h"
#include "sc_st7789_ch32v_hwspi.h"
#include <math.h>
#include <stdio.h>

/// global variables that should be initialzed to the size of the target screen on startup. (before DVP initializing and DVP interrupts)
int camera_screen_width;
int camera_screen_height;

int fancy_display(struct sc_painter *painter)
{
	static int current_cnt = 0, step = 1;
	struct point p;
	struct point size;
	long color;
	int i;

	sc_painter_size(painter, &size);
	point_init(&p, size.x / 2, size.y / 2);
	for (i = 0; i < 31; i++) {
		color = (ABS(current_cnt - i) < 3) ? BLACK_24bit : CYAN_24bit;
		sc_painter_draw_circle(painter, p, i, color);
	}
	sc_painter_flush(painter);

	if (current_cnt == 31)
		step = -1;
	else if (current_cnt == 0)
		step = 1;

	current_cnt += step;
	return 0;
}

int screen_1_init(struct ssd1306_screen *screen, struct ssd1306_adaptor_ch32v_i2c *adaptor)
{
	if (ssd1306_adaptor_ch32v_i2c_init(adaptor, 0x3C))
		return 1;
	if (ssd1306_init(screen, (struct ssd1306_adaptor_i **)adaptor))
		return 2;
	if (ssd1306_display_on(screen))
		return 3;
	return 0;
}

/*
int screen_2_init(struct st7735_screen *screen, struct st7735_adaptor_ch32v_spi *adaptor)
{
	if (st7735_adaptor_ch32v_spi_init(adaptor, ...))
		return 1;
	if (st7735_init(screen, (struct st7735_adaptor_i **)adaptor))
		return 2;
	return 0;
}
*/

int lcd_bg_pwm_init(uint16_t prescale, uint16_t period, uint16_t compare_value)
{
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
	return 0;
}

int lcd_bg_set_brightness(uint16_t brightness)
{
	TIM1->CH2CVR = brightness;
	return 0;
}

int screen_3_init(struct st7789_screen *screen, struct st7789_adaptor_ch32v_fsmc *adaptor)
{
	if (st7789_adaptor_ch32v_fsmc_init(adaptor))
		return 1;
	if (st7789_init(screen, (struct st7789_adaptor_i **)adaptor))
		return 2;

	// GPIO_SetBits(GPIOB, GPIO_Pin_14);

	if (lcd_bg_pwm_init(144 - 1, 100, 50))
		return 3;
	if (lcd_bg_set_brightness(20))
		return 4;
	return 0;
}

int screen_4_init(struct st7789_screen *screen, struct st7789_adaptor_ch32v_hwspi *adaptor)
{
	if (st7789_adaptor_ch32v_hwspi_init(adaptor))
		return 1;
	if (st7789_init(screen, (struct st7789_adaptor_i **)adaptor))
		return 2;

	return 0;
}

int graphic_play(struct sc_painter *painter)
{
	struct point p1, p2, p3, size;
	struct sc_text_painter text_painter;

	sc_painter_clear(painter, BLACK_24bit);

	/// The default method do not flush, but overridden `clear` can do flush automatically.
	// sc_painter_flush(painter);

	/// text drawing
	sc_text_painter_init(&text_painter, painter);

	color_pair_init(&text_painter.color, RED_24bit, BLACK_24bit);
	point_init(&text_painter.pos, 0, 0);

	sc_text_draw_string(&text_painter, "1.5 Programming!", 32);

	color_pair_init(&text_painter.color, GREEN_24bit, BLACK_24bit);
	point_init(&text_painter.pos, 0, 32);

	sc_text_draw_string(&text_painter, "1.5 Programming!", 16);

	sc_painter_size(painter, &size);

	point_init(&p1, size.x / 2 - 50, size.y / 2 - 20);
	point_init(&p2, size.x / 2 + 50, size.y / 2 + 20);
	sc_painter_draw_rectangle(painter, p1, p2, YELLOW_24bit);

	point_init(&p1, size.x / 2 - 50, size.y / 2 - 20);
	sc_painter_draw_circle(painter, p1, 5, MAGENTA_24bit);

	point_init(&p1, 10, size.y / 2 - 20);
	point_init(&p2, 10, size.y / 2 + 20);
	sc_painter_draw_line(painter, p1, p2, WHITE_24bit);

	point_init(&p1, 60, 60);
	point_init(&p2, 10, size.y - 60);
	point_init(&p3, size.x, -60);

	sc_painter_draw_bezier(painter, p1, p2, p3, BLUE_24bit);

	sc_painter_flush(painter);

	while (1)
		fancy_display(painter);

	return 0;
}

int dma_lcd_init(uintptr_t periph_address)
{
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
	return 0;
}

int camera_display(struct sc_painter *painter, struct st7789_screen *screen)
{
	struct point p1, p2, size;

	sc_painter_size(painter, &size);
	/// initialize global variables that represent the size of the screen.
	camera_screen_width = size.x;
	camera_screen_height = size.y;

	sc_painter_clear(painter, BLACK_24bit);
	// sc_painter_clear(painter, RED_24bit);

	// point_init(&p1, 0, 0);
	// point_init(&p2, camera_screen_width - 1, camera_screen_height - 1);

	/// Make sure that `p2.y - p1.y == camera_screen_height - 1`, and `p1.y < 0`.
	point_init(&p1, 0, -1);
	point_init(&p2, camera_screen_width - 1, camera_screen_height - 1 - 1);
	// point_init(&p1, 0, -10);
	// point_init(&p2, camera_screen_width - 1, camera_screen_height - 1 - 10);
	st7789_set_address(screen, p1, p2);

	while (ov2640_init())
		delay_ms(500);

	delay_ms(100);
	ov2640_rgb565_mode_init();
	delay_ms(100);

	dma_lcd_init((uintptr_t)rgb565_dvp_dma_buffer0);
	dvp_init();

	while (1)
		;

	return 0;
}

int compass_display(struct sc_painter *painter)
{
	struct point p1, p2, size, center;
	int r = 50;
	float theta = 45.0 / 180.0 * M_PI;

	sc_painter_clear(painter, BLACK_24bit);

	sc_painter_size(painter, &size);
	center.x = size.x / 2;
	center.y = size.y / 2;

	point_init(&p1, center.x + r * cos(theta), center.y - r * sin(theta));
	sc_painter_draw_line(painter, p1, center, RED_24bit);
	point_init(&p1, center.x - r * cos(theta), center.y + r * sin(theta));
	sc_painter_draw_line(painter, p1, center, BLUE_24bit);
	return 0;
}

int main()
{
	struct ssd1306_adaptor_ch32v_i2c adaptor1;
	struct ssd1306_screen screen1;
	// struct st7735_adaptor_ch32v_spi adaptor2;
	// struct st7735_screen screen2;
	// struct st7789_adaptor_ch32v_fsmc adaptor3;
	// struct st7789_screen screen3;
	struct st7789_adaptor_ch32v_hwspi adaptor4;
	struct st7789_screen screen4;
	struct sc_painter painter;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	systick_interrupt_init();

	// screen_1_init(&screen1, &adaptor1);
	// screen_2_init(&screen2, &adaptor2);
	// screen_3_init(&screen3, &adaptor3);
	screen_4_init(&screen4, &adaptor4);

	// SSD1306_Screen_set_up_down_invert(&screen1);

	// painter.drawing_board = (struct drawing_i **)&screen1;
	// painter.drawing_board = (struct drawing_i **)&screen2;
	// painter.drawing_board = (struct drawing_i **)&screen3;
	painter.drawing_board = (struct drawing_i **)&screen4;

	usart_printf_init(115200);
	printf("System is ready now. SystemClk: %lu\r\n", SystemCoreClock);

	// camera_display(&painter, &screen3);
	graphic_play(&painter);
	// compass_display(&painter);

	while (1)
		;
	return 0;
}
