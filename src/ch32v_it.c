#include "camera_ov2640.h"
#include "ch32v30x.h"
#include <assert.h>

static volatile uint32_t frame_cnt = 0;
static volatile uint32_t addr_cnt = 0;
static volatile uint32_t href_cnt = 0;

extern uint16_t camera_screen_width;

void DVP_RowDoneHandler() {
	uintptr_t dvp_dma_buffer;
	int i, columns;

	/// This function should be called after `camera_screen_width` got initialized.
	//assert(camera_screen_width > 0);

	DVP->IFR &= ~RB_DVP_IF_ROW_DONE;
	DMA_Cmd(DMA2_Channel5, DISABLE);

	if (addr_cnt % 2)
		dvp_dma_buffer = (uintptr_t) RGB565_dvp_dma_buffer0;
	else
		dvp_dma_buffer = (uintptr_t) RGB565_dvp_dma_buffer1;

	/// The size of the buffer is `camera_screen_width * 4`.
	columns = camera_screen_width * 2;

	/// Convert the loosely packed 4-byte RGB565 pixel to 2 bytes by a
	/// shift operation: `>> 2` (Y9-Y2 -> D7-D0)
	for (i = 0; i < columns; i++)
		*(uint8_t *) (dvp_dma_buffer + i) =
			*(uint16_t *) (dvp_dma_buffer + i * 2) >> 2;

	DMA2_Channel5->PADDR = dvp_dma_buffer;
	DMA2_Channel5->CNTR = columns;
	DMA_Cmd(DMA2_Channel5, ENABLE);

	addr_cnt++;
	href_cnt++;
}

void DVP_FrmDoneHandler() {
	DVP->IFR &= ~RB_DVP_IF_FRM_DONE;
	addr_cnt = 0;
	href_cnt = 0;
}

void DVP_StrFrmHandler() {
	DVP->IFR &= ~RB_DVP_IF_STR_FRM;
	frame_cnt++;
}

void DVP_StpFrmHandler() {
	DVP->IFR &= ~RB_DVP_IF_STP_FRM;
}

void DVP_FifoOvHandler() {
	DVP->IFR &= ~RB_DVP_IF_FIFO_OV;
	/// for debug
	while (1);
}

__attribute__((interrupt("WCH-Interrupt-fast")))
void DVP_IRQHandler() {
	if (DVP->IFR & RB_DVP_IF_ROW_DONE) DVP_RowDoneHandler();
	if (DVP->IFR & RB_DVP_IF_FRM_DONE) DVP_FrmDoneHandler();
	if (DVP->IFR & RB_DVP_IF_STR_FRM) DVP_StrFrmHandler();
	if (DVP->IFR & RB_DVP_IF_STP_FRM) DVP_StpFrmHandler();
	if (DVP->IFR & RB_DVP_IF_FIFO_OV) DVP_FifoOvHandler();
}

__attribute__((interrupt("WCH-Interrupt-fast")))
void NMI_Handler() {
}

