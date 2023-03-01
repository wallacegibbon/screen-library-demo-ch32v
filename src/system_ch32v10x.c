#include "ch32v10x.h"

/// If none is enabled, the HSI is used as System clock source.

//#define SYSCLK_FREQ_HSE HSE_VALUE
//#define SYSCLK_FREQ_48MHz_HSE 48000000
//#define SYSCLK_FREQ_56MHz_HSE 56000000
#define SYSCLK_FREQ_72MHz_HSE 72000000
//#define SYSCLK_FREQ_HSI HSI_VALUE
//#define SYSCLK_FREQ_48MHz_HSI 48000000
//#define SYSCLK_FREQ_56MHz_HSI 56000000
//#define SYSCLK_FREQ_72MHz_HSI 72000000

/// Clock Definitions 
#ifdef SYSCLK_FREQ_HSE
uint32_t SystemCoreClock = SYSCLK_FREQ_HSE;
#elif defined SYSCLK_FREQ_48MHz_HSE
uint32_t SystemCoreClock = SYSCLK_FREQ_48MHz_HSE;
#elif defined SYSCLK_FREQ_56MHz_HSE
uint32_t SystemCoreClock = SYSCLK_FREQ_56MHz_HSE;
#elif defined SYSCLK_FREQ_72MHz_HSE
uint32_t SystemCoreClock = SYSCLK_FREQ_72MHz_HSE;
#elif defined SYSCLK_FREQ_48MHz_HSI
uint32_t SystemCoreClock = SYSCLK_FREQ_48MHz_HSI;
#elif defined SYSCLK_FREQ_56MHz_HSI
uint32_t SystemCoreClock = SYSCLK_FREQ_56MHz_HSI;
#elif defined SYSCLK_FREQ_72MHz_HSI
uint32_t SystemCoreClock = SYSCLK_FREQ_72MHz_HSI;
#else
uint32_t SystemCoreClock = HSI_VALUE;

#endif

__I uint8_t AHBPrescTable[16] = {
	0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9
};

static void SetSysClock();

#ifdef SYSCLK_FREQ_HSE
static void SetSysClockToHSE();
#elif defined SYSCLK_FREQ_48MHz_HSE
static void SetSysClockTo48_HSE();
#elif defined SYSCLK_FREQ_56MHz_HSE
static void SetSysClockTo56_HSE();
#elif defined SYSCLK_FREQ_72MHz_HSE
static void SetSysClockTo72_HSE();
#elif defined SYSCLK_FREQ_48MHz_HSI
static void SetSysClockTo48_HSI();
#elif defined SYSCLK_FREQ_56MHz_HSI
static void SetSysClockTo56_HSI();
#elif defined SYSCLK_FREQ_72MHz_HSI
static void SetSysClockTo72_HSI();

#endif

/// Setup the microcontroller system Initialize the Embedded Flash Interface,
/// the PLL and update the SystemCoreClock variable.
void SystemInit() {
	RCC->CTLR |= 0x00000001;
	RCC->CFGR0 &= 0xF8FF0000;
	RCC->CTLR &= 0xFEF6FFFF;
	RCC->CTLR &= 0xFFFBFFFF;
	RCC->CFGR0 &= 0xFF80FFFF;
	RCC->INTR = 0x009F0000;
	SetSysClock();
}

/// Update SystemCoreClock variable according to Clock Register Values.
void SystemCoreClockUpdate() {
	uint32_t tmp, pllmull = 0, pllsource = 0;

	tmp = RCC->CFGR0 & RCC_SWS;

	switch (tmp) {
	case 0x00:
		SystemCoreClock = HSI_VALUE;
		break;
	case 0x04:
		SystemCoreClock = HSE_VALUE;
		break;
	case 0x08:
		pllmull = RCC->CFGR0 & RCC_PLLMULL;
		pllsource = RCC->CFGR0 & RCC_PLLSRC;
		pllmull = (pllmull >> 18) + 2;
		if (pllsource == 0x00) {
			SystemCoreClock = (HSI_VALUE >> 1) * pllmull;
			break;
		}

		if ((RCC->CFGR0 & RCC_PLLXTPRE) != RESET)
			SystemCoreClock = (HSE_VALUE >> 1) * pllmull;
		else
			SystemCoreClock = HSE_VALUE * pllmull;

		break;
	default:
		SystemCoreClock = HSI_VALUE;
		break;
	}

	tmp = AHBPrescTable[(RCC->CFGR0 & RCC_HPRE) >> 4];
	SystemCoreClock >>= tmp;
}

/// Configures the System clock frequency, HCLK, PCLK2 and PCLK1 prescalers.
static void SetSysClock() {
#ifdef SYSCLK_FREQ_HSE
	SetSysClockToHSE();
#elif defined SYSCLK_FREQ_48MHz_HSE
	SetSysClockTo48_HSE();
#elif defined SYSCLK_FREQ_56MHz_HSE
	SetSysClockTo56_HSE();
#elif defined SYSCLK_FREQ_72MHz_HSE
	SetSysClockTo72_HSE();
#elif defined SYSCLK_FREQ_48MHz_HSI
	SetSysClockTo48_HSI();
#elif defined SYSCLK_FREQ_56MHz_HSI
	SetSysClockTo56_HSI();
#elif defined SYSCLK_FREQ_72MHz_HSI
	SetSysClockTo72_HSI();
#endif
	/// If none of the define above is enabled,
	/// the HSI is used as System clock source (default after reset)
}

#ifdef SYSCLK_FREQ_HSE

static void SetSysClockToHSE() {
	__IO uint32_t StartUpCounter = 0, HSEStatus = 0;

	RCC->CTLR |= RCC_HSEON;

	/// Wait till HSE is ready and if Time out is reached exit
	while ((HSEStatus == 0) && (StartUpCounter++ != HSE_STARTUP_TIMEOUT))
		HSEStatus = RCC->CTLR & RCC_HSERDY;

	if ((RCC->CTLR & RCC_HSERDY) != RESET)
		HSEStatus = 1;
	else
		HSEStatus = 0;

	if (HSEStatus != 0x01) {
		/// ...
		return;
	}

	FLASH->ACTLR |= FLASH_ACTLR_PRFTBE;
	/// Flash 0 wait state
	FLASH->ACTLR &= ~FLASH_ACTLR_LATENCY;
	FLASH->ACTLR |= FLASH_ACTLR_LATENCY_0;

	/// HCLK = SYSCLK
	RCC->CFGR0 |= RCC_HPRE_DIV1;
	/// PCLK2 = HCLK
	RCC->CFGR0 |= RCC_PPRE2_DIV1;
	/// PCLK1 = HCLK
	RCC->CFGR0 |= RCC_PPRE1_DIV1;

	/// Select HSE as system clock source
	RCC->CFGR0 &= ~RCC_SW;
	RCC->CFGR0 |= RCC_SW_HSE;

	/// Wait till HSE is used as system clock source
	while ((RCC->CFGR0 & RCC_SWS) != 0x04);
}

#elif defined SYSCLK_FREQ_48MHz_HSE

static void SetSysClockTo48_HSE() {
	__IO uint32_t StartUpCounter = 0, HSEStatus = 0;

	RCC->CTLR |= RCC_HSEON;

	/// Wait till HSE is ready and if Time out is reached exit
	while ((HSEStatus == 0) && (StartUpCounter++ != HSE_STARTUP_TIMEOUT))
		HSEStatus = RCC->CTLR & RCC_HSERDY;

	if ((RCC->CTLR & RCC_HSERDY) != RESET)
		HSEStatus = 1;
	else
		HSEStatus = 0;

	if (HSEStatus != 0x01) {
		 /// ...
		 return;
	}

	/// Enable Prefetch Buffer
	FLASH->ACTLR |= FLASH_ACTLR_PRFTBE;

	/// Flash 1 wait state
	FLASH->ACTLR &= ~FLASH_ACTLR_LATENCY;
	FLASH->ACTLR |= FLASH_ACTLR_LATENCY_1;

	/// HCLK = SYSCLK
	RCC->CFGR0 |= RCC_HPRE_DIV1;
	/// PCLK2 = HCLK
	RCC->CFGR0 |= RCC_PPRE2_DIV1;
	/// PCLK1 = HCLK */
	RCC->CFGR0 |= RCC_PPRE1_DIV2;

	/// PLL configuration: PLLCLK = HSE * 6 = 48 MHz */
	RCC->CFGR0 &= ~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL);
	RCC->CFGR0 |= RCC_PLLSRC_HSE | RCC_PLLMULL6;

	/// Enable PLL
	RCC->CTLR |= RCC_PLLON;

	/// Wait till PLL is ready
	while ((RCC->CTLR & RCC_PLLRDY) == 0);

	/// Select PLL as system clock source
	RCC->CFGR0 &= ~RCC_SW;
	RCC->CFGR0 |= RCC_SW_PLL;

	/// Wait till PLL is used as system clock source
	while ((RCC->CFGR0 & RCC_SWS) != 0x08);
}

#elif defined SYSCLK_FREQ_56MHz_HSE

static void SetSysClockTo56_HSE() {
	__IO uint32_t StartUpCounter = 0, HSEStatus = 0;

	RCC->CTLR |= RCC_HSEON;

	/// Wait till HSE is ready and if Time out is reached exit
	while ((HSEStatus == 0) && (StartUpCounter++ != HSE_STARTUP_TIMEOUT))
		HSEStatus = RCC->CTLR & RCC_HSERDY;

	if ((RCC->CTLR & RCC_HSERDY) != RESET)
		HSEStatus = 1;
	else
		HSEStatus = 0;

	if (HSEStatus != 0x01) {
		/// ...
		return;
	}

	/// Enable Prefetch Buffer
	FLASH->ACTLR |= FLASH_ACTLR_PRFTBE;

	/// Flash 2 wait state
	FLASH->ACTLR &= ~FLASH_ACTLR_LATENCY;
	FLASH->ACTLR |= FLASH_ACTLR_LATENCY_2;

	/// HCLK = SYSCLK
	RCC->CFGR0 |= RCC_HPRE_DIV1;
	/// PCLK2 = HCLK
	RCC->CFGR0 |= RCC_PPRE2_DIV1;
	/// PCLK1 = HCLK
	RCC->CFGR0 |= RCC_PPRE1_DIV2;

	/// PLL configuration: PLLCLK = HSE * 7 = 56 MHz
	RCC->CFGR0 &= ~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL);
	RCC->CFGR0 |= RCC_PLLSRC_HSE | RCC_PLLMULL7;
	/// Enable PLL
	RCC->CTLR |= RCC_PLLON;

	/// Wait till PLL is ready
	while ((RCC->CTLR & RCC_PLLRDY) == 0);

	/// Select PLL as system clock source
	RCC->CFGR0 &= ~RCC_SW;
	RCC->CFGR0 |= RCC_SW_PLL;

	/// Wait till PLL is used as system clock source
	while ((RCC->CFGR0 & RCC_SWS) != 0x08);
}

#elif defined SYSCLK_FREQ_72MHz_HSE

static void SetSysClockTo72_HSE() {
	__IO uint32_t StartUpCounter = 0, HSEStatus = 0;

	RCC->CTLR |= RCC_HSEON;

	/// Wait till HSE is ready and if Time out is reached exit
	while ((HSEStatus == 0) && (StartUpCounter++ != HSE_STARTUP_TIMEOUT))
		HSEStatus = RCC->CTLR & RCC_HSERDY;

	if ((RCC->CTLR & RCC_HSERDY) != RESET)
		HSEStatus = 1;
	else
		HSEStatus = 0;

	if (HSEStatus != 0x01) {
		 /// ...
		return;
	}

	/// Enable Prefetch Buffer
	FLASH->ACTLR |= FLASH_ACTLR_PRFTBE;

	/// Flash 2 wait state
	FLASH->ACTLR &= ~FLASH_ACTLR_LATENCY;
	FLASH->ACTLR |= FLASH_ACTLR_LATENCY_2;

	/// HCLK = SYSCLK
	RCC->CFGR0 |= RCC_HPRE_DIV1;
	/// PCLK2 = HCLK
	RCC->CFGR0 |= RCC_PPRE2_DIV1;
	/// PCLK1 = HCLK
	RCC->CFGR0 |= RCC_PPRE1_DIV2;

	/// PLL configuration: PLLCLK = HSE * 9 = 72 MHz
	RCC->CFGR0 &= ~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL);
	RCC->CFGR0 |= RCC_PLLSRC_HSE | RCC_PLLMULL9;
	/// Enable PLL 
	RCC->CTLR |= RCC_PLLON;

	/// Wait till PLL is ready 
	while ((RCC->CTLR & RCC_PLLRDY) == 0);

	/// Select PLL as system clock source 
	RCC->CFGR0 &= ~RCC_SW;
	RCC->CFGR0 |= RCC_SW_PLL;

	/// Wait till PLL is used as system clock source 
	while ((RCC->CFGR0 & RCC_SWS) != 0x08);
}

#elif defined SYSCLK_FREQ_48MHz_HSI

static void SetSysClockTo48_HSI() {
	EXTEN->EXTEN_CTR |= EXTEN_PLL_HSI_PRE;

	// Enable Prefetch Buffer
	FLASH->ACTLR |= FLASH_ACTLR_PRFTBE;

	/// Flash 1 wait state
	FLASH->ACTLR &= ~FLASH_ACTLR_LATENCY;
	FLASH->ACTLR |= FLASH_ACTLR_LATENCY_1;

	/// HCLK = SYSCLK
	RCC->CFGR0 |= RCC_HPRE_DIV1;
	/// PCLK2 = HCLK
	RCC->CFGR0 |= RCC_PPRE2_DIV1;
	/// PCLK1 = HCLK
	RCC->CFGR0 |= RCC_PPRE1_DIV2;

	/// PLL configuration: PLLCLK = HSI * 6 = 48 MHz
	RCC->CFGR0 &= ~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL);
	RCC->CFGR0 |= RCC_PLLSRC_HSI_Div2 | RCC_PLLMULL6;

	/// Enable PLL 
	RCC->CTLR |= RCC_PLLON;
	/// Wait till PLL is ready 
	while ((RCC->CTLR & RCC_PLLRDY) == 0);

	/// Select PLL as system clock source 
	RCC->CFGR0 &= ~RCC_SW;
	RCC->CFGR0 |= RCC_SW_PLL;

	/// Wait till PLL is used as system clock source 
	while ((RCC->CFGR0 & RCC_SWS) != 0x08);
}

#elif defined SYSCLK_FREQ_56MHz_HSI

/// Sets System clock frequency to 56MHz and configure HCLK,
/// PCLK2 and PCLK1 prescalers.
static void SetSysClockTo56_HSI() {
	EXTEN->EXTEN_CTR |= EXTEN_PLL_HSI_PRE;

	/// Enable Prefetch Buffer 
	FLASH->ACTLR |= FLASH_ACTLR_PRFTBE;

	/// Flash 1 wait state 
	FLASH->ACTLR &= ~FLASH_ACTLR_LATENCY;
	FLASH->ACTLR |= FLASH_ACTLR_LATENCY_1;

	/// HCLK = SYSCLK 
	RCC->CFGR0 |= RCC_HPRE_DIV1;
	/// PCLK2 = HCLK 
	RCC->CFGR0 |= RCC_PPRE2_DIV1;
	/// PCLK1 = HCLK 
	RCC->CFGR0 |= RCC_PPRE1_DIV2;

	/// PLL configuration: PLLCLK = HSI * 7 = 56 MHz 
	RCC->CFGR0 &= ~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL);
	RCC->CFGR0 |= RCC_PLLSRC_HSI_Div2 | RCC_PLLMULL7;

	/// Enable PLL 
	RCC->CTLR |= RCC_PLLON;
	/// Wait till PLL is ready 
	while ((RCC->CTLR & RCC_PLLRDY) == 0);

	/// Select PLL as system clock source 
	RCC->CFGR0 &= ~RCC_SW;
	RCC->CFGR0 |= RCC_SW_PLL;

	/// Wait till PLL is used as system clock source 
	while ((RCC->CFGR0 & RCC_SWS) != 0x08);
}

#elif defined SYSCLK_FREQ_72MHz_HSI

/// Sets System clock frequency to 72MHz and configure HCLK,
/// PCLK2 and PCLK1 prescalers.
static void SetSysClockTo72_HSI() {
	EXTEN->EXTEN_CTR |= EXTEN_PLL_HSI_PRE;

	/// Enable Prefetch Buffer 
	FLASH->ACTLR |= FLASH_ACTLR_PRFTBE;

	/// Flash 1 wait state 
	FLASH->ACTLR &= ~FLASH_ACTLR_LATENCY;
	FLASH->ACTLR |= FLASH_ACTLR_LATENCY_1;

	/// HCLK = SYSCLK 
	RCC->CFGR0 |= RCC_HPRE_DIV1;
	/// PCLK2 = HCLK 
	RCC->CFGR0 |= RCC_PPRE2_DIV1;
	/// PCLK1 = HCLK 
	RCC->CFGR0 |= RCC_PPRE1_DIV2;

	/// PLL configuration: PLLCLK = HSI * 9 = 72 MHz 
	RCC->CFGR0 &= ~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL);
	RCC->CFGR0 |= RCC_PLLSRC_HSI_Div2 | RCC_PLLMULL9;

	/// Enable PLL 
	RCC->CTLR |= RCC_PLLON;

	/// Wait till PLL is ready 
	while ((RCC->CTLR & RCC_PLLRDY) == 0);

	/// Select PLL as system clock source 
	RCC->CFGR0 &= ~RCC_SW;
	RCC->CFGR0 |= RCC_SW_PLL;

	/// Wait till PLL is used as system clock source 
	while ((RCC->CFGR0 & RCC_SWS) != 0x08);
}

#endif

