#include <hal/platform.h>
#include <hal/link.h>
#include <types/basetype.h>

LKHEAD_T static void imx6ull_mmu_set_tbladdr(u32_t tbl_phy_addr)
{
    __asm__ __volatile__(
        //把[tbass]关联的寄存器中的值写入CP15的C2寄存器中
        "mcr p15,0,%[tbass],c2,c0,0 \n\t"
        :
        //分配一个通用寄存器，在其中写入tblbphyadr的内容，该动作是由GCC完成的
        : [tbass]"r"(tbl_phy_addr)
        : "cc","memory"
    );
    return;
}
LKHEAD_T static void imx6ull_mmu_set_domain(u32_t domain)
{
    __asm__ __volatile__(
        //把[domval]关联的寄存器中的值写入CP15的C3寄存器中
        "mcr p15,0,%[domval],c3,c0,0 \n\t"
        : : [domval]"r"(domain)
        : "cc","memory"
    );
    return;
}

LKHEAD_T static void imx6ull_mmu_invalid_dicache()
{
    __asm__ __volatile__(
        "mov    r0, #0 \n\t"
        "mcr    p15, 0, r0, c7, c7, 0 \n\t"     //使指令Cache和数据Cache中的数据无效
        "mcr    p15, 0, r0, c7, c10, 4 \n\t"
        "mcr    p15, 0, r0, c8, c7, 0 \n\t"     //使指令TLB、数据TLB中的数据无效
        : : :"cc","memory","r0"                 //用到了r0寄存器，所示要告诉GCC
    );
    return;
}

LKHEAD_T static void imx6ull_mmu_enable()
{
    __asm__ __volatile__(
        "mrc p15,0,r0,c1,c0,0 \n\t" //把CP15的C1寄存器中的值读取到r0寄存器中
        "orr r0 ,#1 \n\t"           //改变r0寄存器中的第0位的数值为1
        "mcr p15,0,r0,c1,c0,0 \n\t" //把r0寄存器中的值写入CP15的C1寄存器中
        "nop \n\t"
        "nop \n\t"
        "nop \n\t"
        "nop \n\t"
        "nop \n\t"
        "nop \n\t"
        "nop \n\t"
        : : : "r0","cc","memory"    //告诉GCC使用了r0寄存器
    );
    return;
}


LKHEAD_T static void imx6ull_mmu_init()
{
    uint_t paddr = 0;
    u32_t pgdesc = 0;
    u32_t *pgdir_addr = (u32_t*)PAGE_TLB_DIR;       //0x80004000
    for(u32_t i = 0 ; i < PAGE_TLB_SIZE ; i++){     //4096个页表项
        pgdesc = paddr | PTE_SECT_AP | PTE_SECT_DOMAIN | PTE_SECT_NOCW | PTE_SECT_BIT;
        pgdir_addr[i] = pgdesc;
        paddr += 0x100000;
    }

    pgdir_addr[0] = SDRAM_MAPVECTPHY_ADDR | PTE_SECT_AP | PTE_SECT_DOMAIN | PTE_SECT_NOCW | PTE_SECT_BIT;

    imx6ull_mmu_set_tbladdr(PAGE_TLB_DIR);  //把页表首地址写入cp15的C2中
    imx6ull_mmu_set_domain(~0);             //设置域寄存器所有位为1
    imx6ull_mmu_invalid_dicache();          //使数据和指令缓存无效
    // imx6ull_mmu_enable();                   //打开mmu
}

LKHEAD_T static void imx6ull_vector_copy()
{
    u32_t *src_t = (u32_t *)(&__end_lmosem_hal_vector);
    u32_t *src = (u32_t *)(&vector);
    u32_t *dst = (u32_t *)(CPU_VECTOR_VIRADR);

    for(uint_t i = 0 ; i < 4096 ; i++){
        dst[i] = 0;
    }

    for(; src < src_t ; src++,dst++){
        *dst = *src;
    }

    return;
}

LKHEAD_T static void imx6ull_vector_init()
{
    imx6ull_vector_copy();
    return;
}

void init_platform()
{
    // imx6ull_mmu_init();
    imx6ull_vector_init();
    return;
}