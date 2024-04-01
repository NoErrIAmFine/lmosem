#ifndef __INTERRUPT_H
#define __INTERRUPT_H

#include <types/basetype.h>
#include <types/list.h>
#include <types/spinlock_t.h>

#define INT_NUMS 160

#define MINT_OFFSET 0
#define SINT_OFFSET 32
#define EINT_OFFSET 47
#define MINTNR_START (0 + MINT_OFFSET)
#define MINTNR_END (31 + MINT_OFFSET)
#define SINTNR_START (0 + SINT_OFFSET)
#define SINTNR_END (14 + SINT_OFFSET)
#define EINTNR_START (0 + EINT_OFFSET)
#define EINTNR_END (23 + EINT_OFFSET)
#define MINT_FLG 1
#define SINT_FLG 2
#define EINT_FLG 3

struct irq_desc
{
    spinlock_t lock;              // 保护其自身的自旋锁
    struct list_head action_list; // 设备中断服务程序链
    uint_t action_count;          // 设备中断服务程序链个数
    u32_t flag;                   // 相关标志位
    u32_t status;                 // 相关状态位
    u32_t pend_bit;               // 中断源挂起寄存器中的位序
    uint_t irq_no;                // 中断号
    uint_t deep;                  // 保留
    uint_t index;                 // 产生中断的次数
};

void init_interrupt();

#endif // __INTERRUPT_H