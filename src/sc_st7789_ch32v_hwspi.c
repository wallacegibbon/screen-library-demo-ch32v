#include "sc_st7789_ch32v_hwspi.h"
#include "ch32v30x_gpio.h"
#include "ch32v30x_rcc.h"
#include "ch32v30x_spi.h"
#include "sc_common.h"
#include "sc_st7789.h"

static int write_data_16(struct st7789_adaptor_ch32v_hwspi *self, int data);
static int write_data(struct st7789_adaptor_ch32v_hwspi *self, int data);
static int write_cmd(struct st7789_adaptor_ch32v_hwspi *self, int cmd);

static struct st7789_adaptor_i adaptor_interface = {
	.write_data_16 = (st7789_adaptor_write_data_16_fn_t)write_data_16,
	.write_data = (st7789_adaptor_write_data_fn_t)write_data,
	.write_cmd = (st7789_adaptor_write_cmd_fn_t)write_cmd,
};

static int write_data_16(struct st7789_adaptor_ch32v_hwspi *self, int data)
{
	/// CS = 0;
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);

	/// DC = 1;
	GPIO_SetBits(GPIOA, GPIO_Pin_3);

	SPI_I2S_SendData(SPI1, data >> 8);
	SPI_I2S_SendData(SPI1, data);

	/// CS = 1;
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
	return 0;
}

static int write_data(struct st7789_adaptor_ch32v_hwspi *self, int data)
{
	/// CS = 0;
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);

	/// DC = 1;
	GPIO_SetBits(GPIOA, GPIO_Pin_3);

	SPI_I2S_SendData(SPI1, data);

	/// CS = 1;
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
	return 0;
}

static int write_cmd(struct st7789_adaptor_ch32v_hwspi *self, int cmd)
{
	/// CS = 0;
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);

	/// DC = 0;
	GPIO_ResetBits(GPIOA, GPIO_Pin_3);
	SPI_I2S_SendData(SPI1, cmd);

	/// CS = 1;
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
	return 0;
}

int st7789_adaptor_ch32v_hwspi_init(struct st7789_adaptor_ch32v_hwspi *self)
{
	GPIO_InitTypeDef gpio_init = {0};
	SPI_InitTypeDef spi_init = {0};

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);

	gpio_init.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_init);

	gpio_init.GPIO_Pin = GPIO_Pin_6;
	gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_init);

	gpio_init.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
	gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_init);

	spi_init.SPI_Mode = SPI_Mode_Master;
	spi_init.SPI_Direction = SPI_Direction_1Line_Tx;
	spi_init.SPI_DataSize = SPI_DataSize_8b;
	spi_init.SPI_CPOL = SPI_CPOL_Low;
	spi_init.SPI_CPHA = SPI_CPHA_1Edge;
	spi_init.SPI_NSS = SPI_NSS_Soft;
	spi_init.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	spi_init.SPI_FirstBit = SPI_FirstBit_MSB;
	spi_init.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &spi_init);

	SPI_Cmd(SPI1, ENABLE);

	GPIO_ResetBits(GPIOA, GPIO_Pin_2);
	delay(100);
	GPIO_SetBits(GPIOA, GPIO_Pin_2);
	delay(100);

	self->adaptor = &adaptor_interface;
	return 0;
}
