#pragma once
#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <stdint.h>



#define CpuInterruptMask()  uint8_t cpuSR
 
#define CpuWithoutInterrupts()              \
  do {                                  \
    asm (                               \
    "MRS   R0, PRIMASK\n\t"             \
    "CPSID I\n\t"                       \
    "STRB R0, %[output]"                \
    : [output] "=m" (cpuSR) :: "r0");   \
  } while(0)
 
#define CpuRestoreInterrupts()               \
  do{                                   \
    asm (                               \
    "ldrb r0, %[input]\n\t"             \
    "msr PRIMASK,r0;\n\t"               \
    ::[input] "m" (cpuSR) : "r0");      \
  } while(0)

class scopedWithoutInterrupts {
private:
	CpuInterruptMask();
public:
	static int enterCount;

	scopedWithoutInterrupts()
	{
		if (enterCount == 0) {
			CpuWithoutInterrupts();
		}
		enterCount++;
	}
	virtual ~scopedWithoutInterrupts()
	{
		enterCount--;
		CpuRestoreInterrupts();
	}
};


#if 0
// many tests, unsuccessful

#define LOCKED      1
#define UNLOCKED    0

typedef uint32_t mutex_t;

extern void lock_mutex(volatile mutex_t* mutex);
extern void unlock_mutex(volatile mutex_t* mutex);


static __inline__ uint8_t __iCliRetVal(void)
{
	asm volatile("cpsid i");
	return 1;
}

static __inline__ void __set_PRIMASK(uint32_t priMask)
{
	register uint32_t __regPriMask         asm("primask");
	__regPriMask = (priMask);
}

#include <core_cm4.h>
#include <cmsis_gcc.h>
static __inline__ void __iRestore(const uint32_t *__s)
{
	__set_PRIMASK(*((uint32_t *)__s));
	__asm__ volatile("" ::: "memory");
}
#endif
#define ATOMIC_BLOCK() for ( uint32_t PRIMASK_save \
__attribute__((__cleanup__(__iRestore))) = __get_PRIMASK(), __ToDo = __iCliRetVal(); \
__ToDo ; __ToDo = 0 )

/*
usage:
ATOMIC_BLOCK()
{
	instruction 1;
	…
	instruction n;
}
*/
#endif