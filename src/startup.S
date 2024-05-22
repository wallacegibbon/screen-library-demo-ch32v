	.macro		IRQ handler
	.word		\handler
	.weak		\handler
	.set		\handler, default_interrupt_handler
	.endm

	.section .isr_vector, "ax", @progbits
	.global		_start
	.option		norvc

_start:
_vector_base:
	/* vector table overlaps reset address to save memory */
	j		handle_reset
	.word		0
	IRQ		NMI_Handler
	IRQ		HardFault_Handler
	.word		0
	IRQ		Ecall_M_Mode_Handler
	.word		0
	.word		0
	IRQ		Ecall_U_Mode_Handler
	IRQ		Break_Point_Handler
	.word		0
	.word		0
	IRQ		SysTick_Handler
	.word		0
	IRQ		SW_Handler
	.word		0
	/* External Interrupts */
	IRQ		WWDG_IRQHandler
	IRQ		PVD_IRQHandler
	IRQ		TAMPER_IRQHandler
	IRQ		RTC_IRQHandler
	IRQ		FLASH_IRQHandler
	IRQ		RCC_IRQHandler
	IRQ		EXTI0_IRQHandler
	IRQ		EXTI1_IRQHandler
	IRQ		EXTI2_IRQHandler
	IRQ		EXTI3_IRQHandler
	IRQ		EXTI4_IRQHandler
	IRQ		DMA1_Channel1_IRQHandler
	IRQ		DMA1_Channel2_IRQHandler
	IRQ		DMA1_Channel3_IRQHandler
	IRQ		DMA1_Channel4_IRQHandler
	IRQ		DMA1_Channel5_IRQHandler
	IRQ		DMA1_Channel6_IRQHandler
	IRQ		DMA1_Channel7_IRQHandler
	IRQ		ADC1_2_IRQHandler
	IRQ		USB_HP_CAN1_TX_IRQHandler
	IRQ		USB_LP_CAN1_RX0_IRQHandler
	IRQ		CAN1_RX1_IRQHandler
	IRQ		CAN1_SCE_IRQHandler
	IRQ		EXTI9_5_IRQHandler
	IRQ		TIM1_BRK_IRQHandler
	IRQ		TIM1_UP_IRQHandler
	IRQ		TIM1_TRG_COM_IRQHandler
	IRQ		TIM1_CC_IRQHandler
	IRQ		TIM2_IRQHandler
	IRQ		TIM3_IRQHandler
	IRQ		TIM4_IRQHandler
	IRQ		I2C1_EV_IRQHandler
	IRQ		I2C1_ER_IRQHandler
	IRQ		I2C2_EV_IRQHandler
	IRQ		I2C2_ER_IRQHandler
	IRQ		SPI1_IRQHandler
	IRQ		SPI2_IRQHandler
	IRQ		USART1_IRQHandler
	IRQ		USART2_IRQHandler
	IRQ		USART3_IRQHandler
	IRQ		EXTI15_10_IRQHandler
	IRQ		RTCAlarm_IRQHandler
	IRQ		USBWakeUp_IRQHandler
	IRQ		TIM8_BRK_IRQHandler
	IRQ		TIM8_UP_IRQHandler
	IRQ		TIM8_TRG_COM_IRQHandler
	IRQ		TIM8_CC_IRQHandler
	IRQ		RNG_IRQHandler
	IRQ		FSMC_IRQHandler
	IRQ		SDIO_IRQHandler
	IRQ		TIM5_IRQHandler
	IRQ		SPI3_IRQHandler
	IRQ		UART4_IRQHandler
	IRQ		UART5_IRQHandler
	IRQ		TIM6_IRQHandler
	IRQ		TIM7_IRQHandler
	IRQ		DMA2_Channel1_IRQHandler
	IRQ		DMA2_Channel2_IRQHandler
	IRQ		DMA2_Channel3_IRQHandler
	IRQ		DMA2_Channel4_IRQHandler
	IRQ		DMA2_Channel5_IRQHandler
	IRQ		ETH_IRQHandler
	IRQ		ETH_WKUP_IRQHandler
	IRQ		CAN2_TX_IRQHandler
	IRQ		CAN2_RX0_IRQHandler
	IRQ		CAN2_RX1_IRQHandler
	IRQ		CAN2_SCE_IRQHandler
	IRQ		OTG_FS_IRQHandler
	IRQ		USBHSWakeup_IRQHandler
	IRQ		USBHS_IRQHandler
	IRQ		DVP_IRQHandler
	IRQ		UART6_IRQHandler
	IRQ		UART7_IRQHandler
	IRQ		UART8_IRQHandler
	IRQ		TIM9_BRK_IRQHandler
	IRQ		TIM9_UP_IRQHandler
	IRQ		TIM9_TRG_COM_IRQHandler
	IRQ		TIM9_CC_IRQHandler
	IRQ		TIM10_BRK_IRQHandler
	IRQ		TIM10_UP_IRQHandler
	IRQ		TIM10_TRG_COM_IRQHandler
	IRQ		TIM10_CC_IRQHandler
	IRQ		DMA2_Channel6_IRQHandler
	IRQ		DMA2_Channel7_IRQHandler
	IRQ		DMA2_Channel8_IRQHandler
	IRQ		DMA2_Channel9_IRQHandler
	IRQ		DMA2_Channel10_IRQHandler
	IRQ		DMA2_Channel11_IRQHandler

	.section .text.default_interrupt_handler, "ax", @progbits

default_interrupt_handler:
	j		default_interrupt_handler
	/*
	mret
	*/

	.section .text.handle_reset, "ax", @progbits

	.weak		handle_reset
	.align		1
	.option		rvc

handle_reset:
	.option		push
	.option		norelax
	la		gp, __global_pointer$
	.option		pop
1:
	la		sp, _eusrstack
2:
	/* Load data section from flash to RAM */
	la		a0, _data_lma
	la		a1, _data_vma
	la		a2, _edata
	bgeu		a1, a2, 2f
1:
	lw		t0, (a0)
	sw		t0, (a1)
	addi		a0, a0, 4
	addi		a1, a1, 4
	bltu		a1, a2, 1b
2:
	/* Clear bss section */
	la		a0, _sbss
	la		a1, _ebss
	bgeu		a0, a1, 2f
1:
	sw		zero, (a0)
	addi		a0, a0, 4
	bltu		a0, a1, 1b
2:
	li		t0, 0x1f
	csrw		0xbc0, t0

	/* Enable nested and hardware stack */
	li		t0, 0x0b
	csrw		0x804, t0

	/* Enable floating point and interrupt */
	li		t0, 0x6088
	csrs		mstatus, t0

	la		t0, _vector_base
	ori		t0, t0, 3
	csrw		mtvec, t0

	jal		SystemInit
	la		t0, main
	csrw		mepc, t0
	mret

