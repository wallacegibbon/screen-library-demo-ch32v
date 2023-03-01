	.extern SystemInit
	.extern main

	.section .init, "ax", @progbits
	.global _start
	.align 1
_start:
	j handle_reset
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00100073

	.macro IRQ handler
	.word \handler
	.weak \handler
	.set \handler, default_interrupt_handler
	.endm

	.section .vector_table, "ax", @progbits
	.align 1
_vector_base:
	.option norvc;
	j _start
	.word 0
	IRQ NMI_Handler
	IRQ HardFault_Handler
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	IRQ SysTick_Handler
	.word 0
	IRQ SW_Handler
	.word 0
	/* External Interrupts */
	IRQ WWDG_IRQHandler
	IRQ PVD_IRQHandler
	IRQ TAMPER_IRQHandler
	IRQ RTC_IRQHandler
	IRQ FLASH_IRQHandler
	IRQ RCC_IRQHandler
	IRQ EXTI0_IRQHandler
	IRQ EXTI1_IRQHandler
	IRQ EXTI2_IRQHandler
	IRQ EXTI3_IRQHandler
	IRQ EXTI4_IRQHandler
	IRQ DMA1_Channel1_IRQHandler
	IRQ DMA1_Channel2_IRQHandler
	IRQ DMA1_Channel3_IRQHandler
	IRQ DMA1_Channel4_IRQHandler
	IRQ DMA1_Channel5_IRQHandler
	IRQ DMA1_Channel6_IRQHandler
	IRQ DMA1_Channel7_IRQHandler
	IRQ ADC1_2_IRQHandler
	.word 0
	.word 0
	.word 0
	.word 0
	IRQ EXTI9_5_IRQHandler
	IRQ TIM1_BRK_IRQHandler
	IRQ TIM1_UP_IRQHandler
	IRQ TIM1_TRG_COM_IRQHandler
	IRQ TIM1_CC_IRQHandler
	IRQ TIM2_IRQHandler
	IRQ TIM3_IRQHandler
	IRQ TIM4_IRQHandler
	IRQ I2C1_EV_IRQHandler
	IRQ I2C1_ER_IRQHandler
	IRQ I2C2_EV_IRQHandler
	IRQ I2C2_ER_IRQHandler
	IRQ SPI1_IRQHandler
	IRQ SPI2_IRQHandler
	IRQ USART1_IRQHandler
	IRQ USART2_IRQHandler
	IRQ USART3_IRQHandler
	IRQ EXTI15_10_IRQHandler
	IRQ RTCAlarm_IRQHandler
	IRQ USBWakeUp_IRQHandler
	IRQ USBHD_IRQHandler

	.option rvc;

	.section .text.default_interrupt_handler, "ax", @progbits
default_interrupt_handler:
	j default_interrupt_handler
	/*
	mret
	*/

	.section .text.handle_reset,"ax",@progbits
	.weak handle_reset
	.align 1
handle_reset:
	.option push 
	.option norelax 
	la gp, __global_pointer$
	.option pop 
1:
	la sp, _eusrstack 
2:
	/* Load data section from flash to RAM */
	la a0, _data_lma
	la a1, _data_vma
	la a2, _edata
	bgeu a1, a2, 2f
1:
	lw t0, (a0)
	sw t0, (a1)
	addi a0, a0, 4
	addi a1, a1, 4
	bltu a1, a2, 1b
2:
	/* clear bss section */
	la a0, _sbss
	la a1, _ebss
	bgeu a0, a1, 2f
1:
	sw zero, (a0)
	addi a0, a0, 4
	bltu a0, a1, 1b
2:
	/* enable all interrupt */
	li t0, 0x88
	csrs mstatus, t0
	la t0, _vector_base
	ori t0, t0, 1
	csrw mtvec, t0

	/*
	la a0,__libc_fini_array
	call atexit
	call __libc_init_array
	*/

	jal SystemInit
	la t0, main
	csrw mepc, t0
	mret

