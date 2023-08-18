#include <ch32v30x.h>
#include <ch32v_debug.h>

void usart_printf_initialize(uint32_t baudrate) {
	GPIO_InitTypeDef gpio_init;
	USART_InitTypeDef usart_init;

#if (DEBUG == DEBUG_UART1)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	gpio_init.GPIO_Pin = GPIO_Pin_9;
	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &gpio_init);

#elif (DEBUG == DEBUG_UART2)
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	gpio_init.GPIO_Pin = GPIO_Pin_2;
	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &gpio_init);

#elif (DEBUG == DEBUG_UART3)
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	gpio_init.GPIO_Pin = GPIO_Pin_10;
	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &gpio_init);

#endif

	usart_init.USART_BaudRate = baudrate;
	usart_init.USART_WordLength = USART_WordLength_8b;
	usart_init.USART_StopBits = USART_StopBits_1;
	usart_init.USART_Parity = USART_Parity_No;
	usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart_init.USART_Mode = USART_Mode_Tx;

#if (DEBUG == DEBUG_UART1)
	USART_Init(USART1, &usart_init);
	USART_Cmd(USART1, ENABLE);

#elif (DEBUG == DEBUG_UART2)
	USART_Init(USART2, &usart_init);
	USART_Cmd(USART2, ENABLE);

#elif (DEBUG == DEBUG_UART3)
	USART_Init(USART3, &usart_init);
	USART_Cmd(USART3, ENABLE);

#endif
}

__attribute__((used)) int _write(int fd, char *buf, int size) {
	int i;

	for (i = 0; i < size; i++) {
#if (DEBUG == DEBUG_UART1)
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
			;
		USART_SendData(USART1, *buf++);
#elif (DEBUG == DEBUG_UART2)
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
			;
		USART_SendData(USART2, *buf++);
#elif (DEBUG == DEBUG_UART3)
		while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
			;
		USART_SendData(USART3, *buf++);
#endif
	}

	return size;
}

void *_sbrk(ptrdiff_t incr) {
	extern char _end[];
	extern char _heap_end[];
	static char *curbrk = _end;

	if ((curbrk + incr < _end) || (curbrk + incr > _heap_end))
		return NULL - 1;

	curbrk += incr;
	return curbrk - incr;
}
