#ifndef __SC_ST7789_CH32V_HWSPI_H
#define __SC_ST7789_CH32V_HWSPI_H

#include "sc_st7789.h"

struct st7789_adaptor_ch32v_hwspi {
	struct st7789_adaptor_i *adaptor;
};

int st7789_adaptor_ch32v_hwspi_init(struct st7789_adaptor_ch32v_hwspi *self);

#endif