#include <hal/interrupt.h>
#include <hal/global.h>
#include <hal/mach.h>

void init_irq_desc_one(struct irq_desc *intp, u32_t flag, u32_t status, u32_t pend_bit, uint_t irq_no)
{
    spin_lock_init(&intp->lock);
    INIT_LIST_HEAD(&intp->action_list);
    intp->action_count = 0;
    intp->flag = flag;
    intp->status = status;
    intp->pend_bit = pend_bit;
    intp->irq_no = irq_no;
    intp->deep = 0;
    intp->index = 0;
    return;
}

void init_irq_desc(struct mach_desc *machp)
{
    struct irq_desc *irqs = machp->irq_descs;
    uint_t irq_num = machp->irq_desc_count;
    
    //遍历irq_desc数据，进行初始化
    for (uint_t i = 0; i < INT_NUMS; i++)
    {
        init_irq_desc_one(&irqs[i], 0, 0, 0, i);
    }
    
    return;
}

void init_interrupt()
{
    init_irq_desc(&os_mach);
    return;
}