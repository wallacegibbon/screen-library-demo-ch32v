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

void fancy_display(struct Painter *painter) {
	static int current_cnt = 0, step = 1;
	struct Point p;
	struct Point size;
	int color, i;

	painter_size(painter, &size);
	point_initialize(&p, size.x / 2, size.y / 2);
	for (i = 0; i < 31; i++) {
		color = current_cnt == i ? BLACK_16bit : GREEN_16bit;
		painter_draw_circle(painter, p, i, color);
	}
	painter_flush(painter);

	if (current_cnt == 31)
		step = -1;
	else if (current_cnt == 0)
		step = 1;

	current_cnt += step;
}

void initialize_screen_1(struct SSD1306_Screen *screen1, struct SSD1306_ScreenAdaptorCH32VI2C *adaptor1) {
	ssd1306_adaptor_ch32v_i2c_initialize(adaptor1, 0x3C);
	ssd1306_initialize(screen1, (struct SSD1306_ScreenAdaptorInterface **)adaptor1);
	ssd1306_display_on(screen1);
}

/*
void initialize_screen_2(struct ST7735_Screen *screen2, struct ST7735_ScreenAdaptorCH32VSPI *adaptor2) {
	st7735_adaptor_ch32v_spi_initialize(adaptor2, ...);
	st7735_ainitialize(screen2, (struct ST7735_ScreenAdaptorInterface **) adaptor2);
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

void initialize_screen_3(struct ST7789_Screen *screen3, struct ST7789_ScreenAdaptorCH32VFSMC *adaptor3) {
	st7789_screen_ch32v_fsmc_initialize(adaptor3);

	st7789_initialize(screen3, (struct ST7789_ScreenAdaptorInterface **)adaptor3);

	// GPIO_SetBits(GPIOB, GPIO_Pin_14);

	lcd_bg_pwm_initialize(144 - 1, 100, 50);
	lcd_bg_set_brightness(20);
}

void graphic_play(struct Painter *painter) {
	struct Point p1, p2, size;
	struct ColorPair color_pair;

	painter_clear(painter, BLACK_16bit);

	/// The default method do not flush, but some overridden `clear` method do flush automatically.
	// painter_flush(painter);

	painter_size(painter, &size);

	point_initialize(&p1, size.x / 2 - 50, size.y / 2 - 20);
	point_initialize(&p2, size.x / 2 + 50, size.y / 2 + 20);
	painter_draw_rectangle(painter, p1, p2, BLUE_16bit);

	point_initialize(&p1, size.x / 2 - 50, size.y / 2 - 20);
	painter_draw_circle(painter, p1, 5, RED_16bit);

	color_pair_initialize(&color_pair, RED_16bit, BLACK_16bit);
	point_initialize(&p1, 0, 0);
	painter_draw_string(painter, "WX, CHITU !", p1, 32, color_pair);

	point_initialize(&p1, 0, 32);
	painter_draw_string(painter, "WX, CHITU !", p1, 16, color_pair);

	/*
	point_initialize(&p1, 0, 0);
	point_initialize(&p2, 20, 50);
	painter_draw_line(painter, p1, p2, WHITE_16bit);
	*/

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

void camera_display(struct Painter *painter, struct ST7789_Screen *screen) {
	struct Point p1, p2, size;

	painter_size(painter, &size);
	/// initialize global variables that represent the size of the screen.
	camera_screen_width = size.x;
	camera_screen_height = size.y;

	painter_clear(painter, BLACK_16bit);
	// painter_clear(painter, RED_16bit);

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

void compass_display(struct Painter *painter) {
	struct Point p1, p2, size, center;
	int r = 50;
	float theta = 45.0 / 180.0 * M_PI;

	painter_clear(painter, BLACK_16bit);

	painter_size(painter, &size);
	center.x = size.x / 2;
	center.y = size.y / 2;

	point_initialize(&p1, center.x + r * cos(theta), center.y - r * sin(theta));
	painter_draw_line(painter, p1, center, RED_16bit);
	point_initialize(&p1, center.x - r * cos(theta), center.y + r * sin(theta));
	painter_draw_line(painter, p1, center, BLUE_16bit);
}

int main() {
	// struct SSD1306_ScreenAdaptorCH32VI2C adaptor1;
	// struct SSD1306_Screen screen1;
	// struct ST7735_ScreenAdaptorCH32VSPI adaptor2;
	// struct ST7735_Screen screen2;
	struct ST7789_ScreenAdaptorCH32VFSMC adaptor3;
	struct ST7789_Screen screen3;
	struct Painter painter;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	initialize_systick_interrupt();

	// initialize_screen_1(&screen1, &adaptor1);
	// initialize_screen_2(&screen2, &adaptor2);
	initialize_screen_3(&screen3, &adaptor3);

	// SSD1306_Screen_set_up_down_invert(&screen1);

	// painter.drawing_board = (struct DrawingBoardInterface **) &screen1;
	// painter.drawing_board = (struct DrawingBoardInterface **) &screen2;
	painter.drawing_board = (struct DrawingBoardInterface **)&screen3;

	usart_printf_initialize(115200);
	printf("System is ready now. SystemClk: %d\r\n", SystemCoreClock);

	// camera_display(&painter, &screen3);
	graphic_play(&painter);
	// compass_display(&painter);

	return 0;
}
