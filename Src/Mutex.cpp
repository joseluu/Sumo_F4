#include "Mutex.h"
#include "stm32f4xx_hal.h"

#if 0
void lock_mutex(volatile mutex_t* mutex)
{
	int result;
	const uint32_t locked = LOCKED;

	__asm__ __volatile__("@lock_mutex\n"
	"1: LDREX   %[r2], [%[r0]]\n"
	"   CMP     %[r2], %[locked]\n"
	"   BEQ     2f\n"
	"   STREXNE %[r2], %[locked], [%[r0]]\n"
	"   CMP     %[r2], #1\n"
	"   BEQ     1b\n"
	"   DMB\n"
	"   B       3f\n"
	"2: WFE\n"
	"   B       1b\n"
	"3: NOP\n"
	    : [r2] "=r" (result),
		[r0] "=r" (mutex)
	    : [locked] "r" (locked));
}

void unlock_mutex(volatile mutex_t* mutex)
{
	const uint32_t unlocked = UNLOCKED;

	__asm__ __volatile__("@unlock_mutex\n"
	"   DMB\n"
	"   STR %[unlocked], [%[r0]]\n"
	"   DSB\n"
	"   SEV\n"
	    : [r0] "=r" (mutex)
	    : [unlocked] "r" (unlocked));
}

#elsevoid lock_mutex(volatile mutex_t* mutex)
{
	int result;
	const uint32_t locked = LOCKED;

	HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);  
	HAL_NVIC_DisableIRQ(TIM8_UP_TIM13_IRQn);
	HAL_NVIC_DisableIRQ(TIM1_TRG_COM_TIM11_IRQn);
	HAL_NVIC_DisableIRQ(TIM1_UP_TIM10_IRQn);
	HAL_NVIC_DisableIRQ(TIM8_TRG_COM_TIM14_IRQn);
	HAL_NVIC_DisableIRQ(TIM8_UP_TIM13_IRQn) ;
	while(*mutex) {
	}
	*mutex = 1;
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn) ;  
	HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn) ;
	HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn) ;
	HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn) ;
	HAL_NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn) ;
	HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn) ;
}

void unlock_mutex(volatile mutex_t* mutex)
{
	*mutex = 0;
}

void get_lock(volatile uint32_t *Lock_Variable)
{ // Note: __LDREXW and __STREXW are CMSIS functions
	int status = 0;
	do {
		while (__LDREXW(Lock_Variable) != 0)
			; // Wait until
		// Lock_Variable is free
		status = __STREXW(1, Lock_Variable); // Try to set
		// Lock_Variable
	} while (status != 0); //retry until lock successfully
	__DMB();		// Do not start any other memory access
	// until memory barrier is completed
	return;
}

void free_lock(volatile int *Lock_Variable)
{ // Note: __LDREXW and __STREXW are CMSIS functions
	__DMB(); // Ensure memory operations completed before
	// releasing lock
	*Lock_Variable = 0;
	return;
}

#endif