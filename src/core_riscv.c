#include <stdint.h>

/// define compiler specific symbols 
#if defined(__CC_ARM)
#define __ASM __asm
#define __INLINE __inline

#elif defined(__ICCARM__)
#define __ASM __asm
#define __INLINE inline

#elif defined(__GNUC__)
#define __ASM __asm
#define __INLINE inline

#elif defined(__TASKING__)
#define __ASM __asm
#define __INLINE inline

#endif

uint32_t __get_MSTATUS(void) {
	uint32_t result;

	__ASM volatile ("csrr %0," "mstatus":"=r" (result));
	return result;
}

void __set_MSTATUS(uint32_t value) {
	__ASM volatile ("csrw mstatus, %0"::"r" (value));
}

uint32_t __get_MISA(void) {
	uint32_t result;

	__ASM volatile ("csrr %0," "misa":"=r" (result));
	return result;
}

void __set_MISA(uint32_t value) {
	__ASM volatile ("csrw misa, %0"::"r" (value));
}

uint32_t __get_MTVEC(void) {
	uint32_t result;

	__ASM volatile ("csrr %0," "mtvec":"=r" (result));
	return result;
}

void __set_MTVEC(uint32_t value) {
	__ASM volatile ("csrw mtvec, %0"::"r" (value));
}

uint32_t __get_MSCRATCH(void) {
	uint32_t result;

	__ASM volatile ("csrr %0," "mscratch":"=r" (result));
	return result;
}

void __set_MSCRATCH(uint32_t value) {
	__ASM volatile ("csrw mscratch, %0"::"r" (value));
}

uint32_t __get_MEPC(void) {
	uint32_t result;

	__ASM volatile ("csrr %0," "mepc":"=r" (result));
	return result;
}

void __set_MEPC(uint32_t value) {
	__ASM volatile ("csrw mepc, %0"::"r" (value));
}

uint32_t __get_MCAUSE(void) {
	uint32_t result;

	__ASM volatile ("csrr %0," "mcause":"=r" (result));
	return result;
}

void __set_MCAUSE(uint32_t value) {
	__ASM volatile ("csrw mcause, %0"::"r" (value));
}

uint32_t __get_MTVAL(void) {
	uint32_t result;

	__ASM volatile ("csrr %0," "mtval":"=r" (result));
	return result;
}

void __set_MTVAL(uint32_t value) {
	__ASM volatile ("csrw mtval, %0"::"r" (value));
}

uint32_t __get_MVENDORID(void) {
	uint32_t result;

	__ASM volatile ("csrr %0," "mvendorid":"=r" (result));
	return result;
}

uint32_t __get_MARCHID(void) {
	uint32_t result;

	__ASM volatile ("csrr %0," "marchid":"=r" (result));
	return result;
}

uint32_t __get_MIMPID(void) {
	uint32_t result;

	__ASM volatile ("csrr %0," "mimpid":"=r" (result));
	return result;
}

uint32_t __get_MHARTID(void) {
	uint32_t result;

	__ASM volatile ("csrr %0," "mhartid":"=r" (result));
	return result;
}

uint32_t __get_SP(void) {
	uint32_t result;

	__ASM volatile ("mv %0," "sp":"=r" (result):);
	return result;
}

