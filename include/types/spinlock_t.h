#ifndef __SPINLOCK_T_H
#define __SPINLOCK_T_H

#include <hal/imx6ul/cpu_ctrl.h>
#include <stdio.h>

typedef struct
{
    volatile u32_t lock;
} spinlock_t;

#define CIRQFIQ 0xc0

#define __ARCH_SPIN_LOCK_UNLOCKED__ 0
#define __ARCH_SPIN_LOCK_LOCKED__ 1

static inline void spin_lock_init(spinlock_t *lock)
{
    lock->lock = __ARCH_SPIN_LOCK_UNLOCKED__;
    return;
}

static inline void spin_lock(spinlock_t *lock)
{
    unsigned long tmp;
    
    __asm__ __volatile__(
        "1: ldrex %0,[%1] \n\t"
        "teq %0,#0 \n\t"
        "strexeq %0,%2,[%1] \n\t"
        "teqeq %0,#0 \n\t"
        "bne 1b \n\t"
        : "=&r"(tmp)
        : "r"(&lock->lock), "r"(1)
        : "cc");
    
    return;
}

static inline void spin_unlock(spinlock_t *lock)
{
    __asm__ __volatile__(
        "str %1,[%0] \n\t"
        :
        : "r"(&lock->lock), "r"(0)
        : "memory");
    return;
}

static inline void spin_lock_irq(spinlock_t *lock)
{
    __asm__ __volatile__(
        "cpsrd i"
        :
        :
        : "memory", "cc");

    spin_lock(lock);
}

static inline void spin_unlock_irq(spinlock_t *lock)
{
    spin_unlock(lock);

    __asm__ __volatile__(
        "cpsre i"
        :
        :
        : "memory", "cc");
}

static inline void spin_lock_irqsave(spinlock_t *lock, cpuflg_t *flag)
{
    cpuflg_t tmpcpsr;
    __asm__ __volatile__(
        "mrs r5,cpsr \n\t"
        "mov %[tmpcpr],r5 \n\t" // 保存CPSR寄存器的值到tmpcpsr变量中
        "orr r5,r5,%[closefirq] \n\t"
        "msr cpsr,r5 \n\t" // 关掉IRQ、FRQ中断
        : [tmpcpr] "=r"(tmpcpsr) : [closefirq] "I"(CIRQFIQ)
        : "r5", "cc", "memory");
    *flag = tmpcpsr; // 把原来的CPSR寄存器的值保存到cpuflg指向的内存单元中
    
    spin_lock(lock);
}

static inline void spin_unlock_irqrestore(spinlock_t *lock, cpuflg_t *flag)
{
    spin_unlock(lock);

    __asm__ __volatile__(
        "msr cpsr,%[ordcpr] \n\t" // 恢复保存在cpuflg地址处的值到CPSR寄存器中
        ::[ordcpr] "r"(*flag)
        : "cc", "memory");
}

#endif // __SPINLOCK_T_H