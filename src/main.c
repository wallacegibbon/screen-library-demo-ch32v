#include "sc_st7789_ch32v_fsmc.h"
#include "sc_st7789.h"
//#include "sc_ssd1306_ch32v_i2c.h"
//#include "sc_ssd1306.h"
//#include "sc_st7735_ch32v_spi.h"
//#include "sc_st7735.h"
#include "sc_painter.h"
#include "sc_common.h"
#include "sc_color.h"
#include "core_systick.h"
#include "ch32v_debug.h"
#include "ch32v30x.h"
#include <stdio.h>

void fancy_display(struct Painter *painter) {
	static int current_cnt = 0, step = 1;
	struct Point p;
	struct Point size;
	int color, i;

	Painter_size(painter, &size);
	Point_initialize(&p, size.x / 2, size.y / 2);
	for (i = 0; i < 31; i++) {
		color = current_cnt == i ? BLACK_16bit : GREEN_16bit;
		Painter_draw_circle(painter, p, i, color);
	}
	Painter_flush(painter);

	if (current_cnt == 31)
		step = -1;
	else if (current_cnt == 0)
		step = 1;

	current_cnt += step;
}

/*
void initialize_screen_1(
	struct SSD1306_Screen *screen1,
	struct SSD1306_ScreenAdaptorCH32VI2C *adaptor1
) {
	SSD1306_ScreenAdaptorCH32VI2C_initialize(
		adaptor1, 0x3C
	);

	SSD1306_Screen_initialize(
		screen1,
		(struct SSD1306_ScreenAdaptorInterface **) adaptor1
	);

	SSD1306_Screen_display_on(screen1);
}

void initialize_screen_2(
	struct ST7735_Screen *screen2,
	struct ST7735_ScreenAdaptorCH32VSPI *adaptor2
) {

	ST7735_ScreenAdaptorCH32VSPI_initialize(
		adaptor2, ...
	);

	ST7735_Screen_initialize(
		screen2,
		(struct ST7735_ScreenAdaptorInterface **) adaptor2
	);
}
*/

void initialize_screen_3(
	struct ST7789_Screen *screen3,
	struct ST7789_ScreenAdaptorCH32VFSMC *adaptor3
) {
	GPIO_InitTypeDef GPIO_InitStructure = { 0 };

	ST7789_ScreenAdaptorCH32VFSMC_initialize(
		adaptor3
	);

	ST7789_Screen_initialize(
		screen3,
		(struct ST7789_ScreenAdaptorInterface **) adaptor3
	);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/// BG LED
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB, GPIO_Pin_14);
}

void main() {
	//struct SSD1306_ScreenAdaptorCH32VI2C adaptor1;
	//struct SSD1306_Screen screen1;
	//struct ST7735_ScreenAdaptorCH32VSPI adaptor2;
	//struct ST7735_Screen screen2;
	struct ST7789_ScreenAdaptorCH32VFSMC adaptor3;
	struct ST7789_Screen screen3;
	struct Painter painter;
	struct Point p1;
	struct Point p2;
	struct Point size;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	initialize_systick_interrupt();

	//USART_printf_initialize(9600);
	//printf("System is ready now. SystemClk: %d\r\n", SystemCoreClock);

	//initialize_screen_1(&screen1, &adaptor1);
	//initialize_screen_2(&screen2, &adaptor2);
	initialize_screen_3(&screen3, &adaptor3);

	//SSD1306_Screen_set_up_down_invert(&screen1);

	//painter.drawing_board = (struct DrawingBoardInterface **) &screen1;
	//painter.drawing_board = (struct DrawingBoardInterface **) &screen2;
	painter.drawing_board = (struct DrawingBoardInterface **) &screen3;

	Painter_clear(&painter, BLACK_16bit);

	/// The default method do not flush, but some overriding `clear` method
	// do flush automatically.
	//Painter_flush(&painter);

	Painter_size(&painter, &size);

	Point_initialize(&p1, size.x / 2 - 50, size.y / 2 - 20);
	Point_initialize(&p2, size.x / 2 + 50, size.y / 2 + 20);
	Painter_draw_rectangle(&painter, p1, p2, BLUE_16bit);

	Point_initialize(&p1, size.x / 2 - 50, size.y / 2 - 20);
	Painter_draw_circle(&painter, p1, 5, RED_16bit);

	/*
	Point_initialize(&p1, 0, 0);
	Point_initialize(&p2, 20, 50);
	Painter_draw_line(&painter, p1, p2, WHITE_16bit);
	*/

	Painter_flush(&painter);

	while (1) {
		fancy_display(&painter);
	}
}

