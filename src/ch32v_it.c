#include "camera_ov2640.h"
#include "ch32v30x.h"

static volatile uint32_t frame_cnt = 0;
static volatile uint32_t addr_cnt = 0;
static volatile uint32_t href_cnt = 0;

void DVP_RowDoneHandler() {
	uintptr_t dvp_dma_addr;
	int i, columns;

	DVP->IFR &= ~RB_DVP_IF_ROW_DONE;
	DMA_Cmd(DMA2_Channel5, DISABLE);

	if (addr_cnt % 2)
		dvp_dma_addr = (uintptr_t) RGB565_DVPDMAaddr0;
	else
		dvp_dma_addr = (uintptr_t) RGB565_DVPDMAaddr1;

	/// columns is defined by the size of the LCD.
	columns = 240 * 2;

	/// In RGB565 mode, every pixel takes 2 bytes. But the DVP of MCU
	/// works in 10-bit mode, every 10-bit data takes 2 bytes,
	/// so every pixel takes 4 bytes in this program.
	/// Convert the 4 bytes to 2 bytes with a `>> 2` (Y9-Y2 -> D7-D0)
	for (i = 0; i < columns; i++)
		*(uint8_t *) (dvp_dma_addr + i) =
			*(uint16_t *) (dvp_dma_addr + i * 2) >> 2;

	DMA2_Channel5->PADDR = dvp_dma_addr;
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

