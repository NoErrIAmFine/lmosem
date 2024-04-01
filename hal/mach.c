#include <hal/mach.h>
#include <hal/global.h>
#include <hal/link.h>
#include <hal/platform.h>

static void mach_init(struct mach_desc *mach_desc)
{
    spin_lock_init(&mach_desc->lock);
    INIT_LIST_HEAD(&mach_desc->list);
    mach_desc->kmem_start = KRNL_INRAM_START;
    mach_desc->kmem_end = (adr_t)(&__end_kernel);
    mach_desc->mem_descs = (struct mem_desc *)(ALIGN(((uint_t)(&__end_kernel)),4096));
    mach_desc->mem_desc_count = 0;
    mach_desc->addr_spaces = mach_addr_space;
    mach_desc->addr_space_count = ADDR_SPACE_NR;
    mach_desc->irq_descs = mach_interrupt;
    mach_desc->irq_desc_count = INT_NUMS;
}

void init_mach()
{
    mach_init(&os_mach);
    return;
}