#ifndef __MACH_T_H
#define __MACH_T_H

#include <types/basetype.h>
#include <types/spinlock_t.h>
#include <types/list.h>
#include <hal/mm.h>
#include <hal/interrupt.h>

struct mach_desc
{
    spinlock_t lock;                    // 保护数据结构自身的自旋锁
    struct list_head list;              // 链表
    adr_t kmem_start;                   // 内核在内存中的开始地址
    adr_t kmem_end;                     // 内核在内存中的结束地址
    struct mem_desc *mem_descs;         // 内存位图在内存中的开始地址
    uint_t mem_desc_count;              // 内存位图的个数
    struct phy_addr_space *addr_spaces; // 物理地址空间描述符的地址
    uint_t addr_space_count;            // 物理地址空间描述符的个数
    // ilnedsc_t* ilnedsc;
    // uint_t     ilnedscnr;
    struct irq_desc *irq_descs; // 中断源描述符地址
    uint_t irq_desc_count;      // 中断源描述符个数
};

void init_mach();

#endif // __MACH_T_H