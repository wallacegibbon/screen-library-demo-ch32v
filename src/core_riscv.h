#ifndef __CORE_RISCV_H
#define __CORE_RISCV_H

/// IO definitions
#ifdef __cplusplus
#define __I volatile
#else
#define __I volatile const
#endif

#define __O volatile
#define __IO volatile

/// Standard Peripheral Library old types (maintained for legacy purpose)
typedef __I uint32_t vuc32;
typedef __I uint16_t vuc16;
typedef __I uint8_t vuc8;

typedef const uint32_t uc32;
typedef const uint16_t uc16;
typedef const uint8_t uc8;

typedef __I int32_t vsc32;
typedef __I int16_t vsc16;
typedef __I int8_t vsc8;

typedef const int32_t sc32;
typedef const int16_t sc16;
typedef const int8_t sc8;

typedef __IO uint32_t vu32;
typedef __IO uint16_t vu16;
typedef __IO uint8_t vu8;

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef __IO int32_t vs32;
typedef __IO int16_t vs16;
typedef __IO int8_t vs8;

typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

typedef enum { NoREADY = 0, READY = !NoREADY } ErrorStatus;

typedef enum { DISABLE = 0, ENABLE = !DISABLE } FunctionalState;

typedef enum { RESET = 0, SET = !RESET } FlagStatus, ITStatus;

#define RV_STATIC_INLINE static inline

/// memory mapped structure for Program Fast Interrupt Controller (PFIC)
typedef struct {
	__I uint32_t ISR[8];
	__I uint32_t IPR[8];
	__IO uint32_t ITHRESDR;
	__IO uint32_t FIBADDRR;
	__IO uint32_t CFGR;
	__I uint32_t GISR;
	uint8_t RESERVED0[0x10];
	__IO uint32_t FIOFADDRR[4];
	uint8_t RESERVED1[0x90];
	__O uint32_t IENR[8];
	uint8_t RESERVED2[0x60];
	__O uint32_t IRER[8];
	uint8_t RESERVED3[0x60];
	__O uint32_t IPSR[8];
	uint8_t RESERVED4[0x60];
	__O uint32_t IPRR[8];
	uint8_t RESERVED5[0x60];
	__IO uint32_t IACTR[8];
	uint8_t RESERVED6[0xE0];
	__IO uint8_t IPRIOR[256];
	uint8_t RESERVED7[0x810];
	__IO uint32_t SCTLR;
} PFIC_Type;

/// memory mapped structure for SysTick 
typedef struct {
	__IO uint32_t CTLR;
	__IO uint8_t CNTL0;
	__IO uint8_t CNTL1;
	__IO uint8_t CNTL2;
	__IO uint8_t CNTL3;
	__IO uint8_t CNTH0;
	__IO uint8_t CNTH1;
	__IO uint8_t CNTH2;
	__IO uint8_t CNTH3;
	__IO uint8_t CMPLR0;
	__IO uint8_t CMPLR1;
	__IO uint8_t CMPLR2;
	__IO uint8_t CMPLR3;
	__IO uint8_t CMPHR0;
	__IO uint8_t CMPHR1;
	__IO uint8_t CMPHR2;
	__IO uint8_t CMPHR3;
} SysTick_Type;

#define PFIC ((PFIC_Type *) 0xE000E000 )
#define NVIC PFIC
#define NVIC_KEY1 ((uint32_t)0xFA050000)
#define	NVIC_KEY2 ((uint32_t)0xBCAF0000)
#define	NVIC_KEY3 ((uint32_t)0xBEEF0000)

#define SysTick ((SysTick_Type *) 0xE000F000)

RV_STATIC_INLINE void __NOP() {
	__asm volatile ("nop");
}

RV_STATIC_INLINE void NVIC_EnableIRQ(IRQn_Type IRQn) {
	NVIC->IENR[(uint32_t) IRQn >> 5] = 1 << ((uint32_t) IRQn & 0x1F);
}

RV_STATIC_INLINE void NVIC_DisableIRQ(IRQn_Type IRQn) {
	uint32_t t;

	t = NVIC->ITHRESDR;
	NVIC->ITHRESDR = 0x10;
	NVIC->IRER[((uint32_t) IRQn >> 5)] = (1 << ((uint32_t) IRQn & 0x1F));
	NVIC->ITHRESDR = t;
}

RV_STATIC_INLINE uint32_t NVIC_GetStatusIRQ(IRQn_Type IRQn) {
	if (NVIC->ISR[(uint32_t) IRQn >> 5] & (1 << ((uint32_t) IRQn & 0x1F)))
		return 1;
	else
		return 0;
}

RV_STATIC_INLINE uint32_t NVIC_GetPendingIRQ(IRQn_Type IRQn) {
	if (NVIC->IPR[(uint32_t) IRQn >> 5] & (1 << ((uint32_t) IRQn & 0x1F)))
		return 1;
	else
		return 0;
}

RV_STATIC_INLINE void NVIC_SetPendingIRQ(IRQn_Type IRQn) {
	NVIC->IPSR[(uint32_t) IRQn >> 5] = 1 << ((uint32_t) IRQn & 0x1F);
}

RV_STATIC_INLINE void NVIC_ClearPendingIRQ(IRQn_Type IRQn) {
	NVIC->IPRR[(uint32_t) IRQn >> 5] = 1 << ((uint32_t) IRQn & 0x1F);
}

RV_STATIC_INLINE uint32_t NVIC_GetActive(IRQn_Type IRQn) {
	if (NVIC->IACTR[(uint32_t) IRQn >> 5] & (1 << ((uint32_t) IRQn & 0x1F)))
		return 1;
	else
		return 0;
}

RV_STATIC_INLINE void NVIC_SetPriority(IRQn_Type IRQn, uint8_t priority) {
	NVIC->IPRIOR[(uint32_t) IRQn] = priority;
}

__attribute__((always_inline))
RV_STATIC_INLINE void __WFI() {
	/// wfi
	NVIC->SCTLR &= ~(1 << 3);
	asm volatile ("wfi");
}

__attribute__((always_inline))
RV_STATIC_INLINE void __WFE() {
	/// (wfi->wfe) + (__sev)
	NVIC->SCTLR |= (1 << 3) | (1 << 5);
	asm volatile ("wfi");
	NVIC->SCTLR |= (1 << 3);
	asm volatile ("wfi");
}

RV_STATIC_INLINE void NVIC_SetFastIRQ(
	uint32_t addr, IRQn_Type IRQn, uint8_t num
) {
	if (num > 3) return;
	NVIC->FIBADDRR = addr;
	NVIC->FIOFADDRR[num] = ((uint32_t) IRQn << 24) | (addr & 0xfffff);
}

RV_STATIC_INLINE void NVIC_SystemReset() {
	NVIC->CFGR = NVIC_KEY3 | (1 << 7);
}

RV_STATIC_INLINE void NVIC_HaltPushCfg(FunctionalState NewState) {
	if (NewState != DISABLE)
		NVIC->CFGR = NVIC_KEY1;
	else
		NVIC->CFGR = NVIC_KEY1 | (1 << 0);
}

RV_STATIC_INLINE void NVIC_INTNestCfg(FunctionalState NewState) {
	if (NewState != DISABLE)
		NVIC->CFGR = NVIC_KEY1;
	else
		NVIC->CFGR = NVIC_KEY1 | (1 << 1);
}

uint32_t __get_MSTATUS();
void __set_MSTATUS(uint32_t value);
uint32_t __get_MISA();
void __set_MISA(uint32_t value);
uint32_t __get_MTVEC();
void __set_MTVEC(uint32_t value);
uint32_t __get_MSCRATCH();
void __set_MSCRATCH(uint32_t value);
uint32_t __get_MEPC();
void __set_MEPC(uint32_t value);
uint32_t __get_MCAUSE();
void __set_MCAUSE(uint32_t value);
uint32_t __get_MTVAL();
void __set_MTVAL(uint32_t value);
uint32_t __get_MVENDORID();
uint32_t __get_MARCHID();
uint32_t __get_MIMPID();
uint32_t __get_MHARTID();
uint32_t __get_SP();

#endif
