#include "ssd1306_ch32v10x_i2c.h"
#include "ssd1306.h"
//#include "st7735_ch32v10x_spi.h"
//#include "st7735.h"
#include "painter.h"
#include "common.h"
#include "color.h"
#include "ch32v_debug.h"
#include <stdio.h>

void fancy_display_1(struct Painter *painter) {
	static int current_cnt = 0, step = 1;
	struct Point p;
	int color, i;

	Point_initialize(&p, 64, 32);
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

void initialize_screen_1(
	struct SSD1306_Screen *screen1,
	struct SSD1306_ScreenAdaptorCH32V10xI2C *adaptor1
) {
	printf("initializing SSD1306...\r\n");

	SSD1306_ScreenAdaptorCH32V10xI2C_initialize(
		adaptor1, 0x3C
	);

	SSD1306_Screen_initialize(
		screen1,
		(struct SSD1306_ScreenAdaptorInterface *) adaptor1
	);

	printf("SSD1306 screen on...\r\n");
	SSD1306_Screen_display_on(screen1);
}

/*
void initialize_screen_2(
	struct ST7735_Screen *screen2,
	struct ST7735_ScreenAdaptorCH32V10xSPI *adaptor2
) {
	printf("initializing ST7735...\r\n");

	ST7735_ScreenAdaptorCH32V10xSPI_initialize(
		adaptor2, ...
	);

	ST7735_Screen_initialize(
		screen2,
		(struct ST7735_ScreenAdaptorInterface *) adaptor2
	);
}
*/

void main() {
	struct SSD1306_ScreenAdaptorCH32V10xI2C adaptor1;
	struct SSD1306_Screen screen1;
	//struct ST7735_ScreenAdaptorCH32V10xSPI adaptor2;
	//struct ST7735_Screen screen2;
	struct Painter painter;
	struct Point p1;
	struct Point p2;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	Delay_Init();
	USART_Printf_Init(9600);
	printf("System is ready now. SystemClk: %d\r\n", SystemCoreClock);

	initialize_screen_1(&screen1, &adaptor1);
	//initialize_screen_2(&screen2, &adaptor2);

	SSD1306_Screen_set_up_down_invert(&screen1);

	painter.screen = (struct DrawingBoardInterface *) &screen1;
	//painter.screen = (struct DrawingBoardInterface *) &screen2;

	printf("clearing screen...\r\n");
	Painter_clear(&painter, BLACK_16bit);

	printf("drawing a rectangle...\r\n");
	Point_initialize(&p1, 64 - 50, 32 - 20);
	Point_initialize(&p2, 64 + 50, 32 + 20);
	Painter_draw_rectangle(&painter, p1, p2, BLUE_16bit);

	printf("drawing a circle on top left...\r\n");
	Point_initialize(&p1, 64 - 50, 32 - 20);
	Painter_draw_circle(&painter, p1, 5, RED_16bit);

	/*
	printf("drawing a line...\r\n");
	Point_initialize(&p1, 30, 10);
	Point_initialize(&p2, 20, 50);
	Painter_draw_line(&painter, p1, p2, WHITE_1bit);
	*/

	printf("flushing the screen...\r\n");
	Painter_flush(&painter);

	while (1) {
		fancy_display_1(&painter);
	}
}

