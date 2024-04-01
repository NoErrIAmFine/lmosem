#include <hal/mm.h>
#include <hal/mach.h>
#include <hal/global.h>
#include <hal/imx6ul/cpu_ctrl.h>
#include <stdio.h>

static void mem_desc_t_init(struct mem_desc *desc, adr_t start_addr, adr_t end_addr, u32_t all_count, u32_t flag)
{
    INIT_LIST_HEAD(&desc->list);
    spin_lock_init(&desc->lock);
    desc->start_addr = start_addr; // 内存块的开始地址
    desc->end_addr = end_addr;     // 内存块的结束地址
    desc->map_count = all_count;   // 分配计数，初始化时为0
    desc->flag = flag;             // 相关的标志
    return;
}

static uint_t init_core_mem_desc(adr_t start_addr, adr_t end_addr, struct mem_desc *descs, uint_t cur_index)
{
    uint_t index = cur_index;
    adr_t tmp_adrr = end_addr;
    for (; start_addr < end_addr; start_addr += MAPONE_SIZE, index++)
    {
        if ((start_addr + MAPONE_SIZE) < end_addr)
        {
            tmp_adrr = (start_addr + MAPONE_SIZE) - 1;
        }
        else
        {
            tmp_adrr = end_addr;
        }
        mem_desc_t_init(&descs[index], start_addr, tmp_adrr, 0, MAP_FLAGES_VAL(0, MAPF_ACSZ_4MB, MAPF_SZ_4MB));
    }
    return index;
}

static void init_mem_desc(struct mach_desc *machp)
{
    struct phy_addr_space *spaces = machp->addr_spaces;
    uint_t space_count = machp->addr_space_count;
    uint_t index = 0;
    for (uint_t i = 0; i < space_count; i++)
    {
        if (spaces[i].flag == ADDR_SPACE_SDRAM)
        {
            index = init_core_mem_desc(spaces[i].addr_start, spaces[i].addr_end, machp->mem_descs, index);
        }
    }
    machp->mem_desc_count = index;
    machp->kmem_end = (adr_t)((uint_t)machp->mem_descs + ((sizeof(struct mem_desc)) * index));
    return;
}

static void mem_slab_init(struct mem_slab *slabp, size_t size)
{
    spin_lock_init(&slabp->lock);
    slabp->size = size;
    INIT_LIST_HEAD(&slabp->full_list);
    INIT_LIST_HEAD(&slabp->empty_list);
    INIT_LIST_HEAD(&slabp->partial_list);
    return;
}

static void phy_mem_init(struct phy_mem *phyp)
{
    INIT_LIST_HEAD(&phyp->list);
    spin_lock_init(&phyp->lock);
    mem_slab_init(&phyp->slabs[0], BLK128KB_SIZE);
    mem_slab_init(&phyp->slabs[1], BLK256KB_SIZE);
    mem_slab_init(&phyp->slabs[2], BLK512KB_SIZE);
    mem_slab_init(&phyp->slabs[3], BLK1MB_SIZE);
    mem_slab_init(&phyp->slabs[4], BLK2MB_SIZE);
    mem_slab_init(&phyp->slabs[5], BLK4MB_SIZE);
    return;
}

static void add_mem_to_slab(struct mem_slab *slabp, struct mem_desc *memp, uint_t flag)
{

    if (!list_empty(&memp->list))
    {
        list_del_init(&memp->list);
    }
    switch (flag)
    {
    case ADD_EMPTY_FLAG:
        list_add_tail(&memp->list, &slabp->empty_list);
        break;
    case ADD_FULL_FLAG:
        list_add_tail(&memp->list, &slabp->full_list);
        break;
    case ADD_PARTIAL_FLAG:
        list_add_tail(&memp->list, &slabp->partial_list);
        break;
    default:
        break;
    }
    return;
}

static void mem_list_init(struct mach_desc *machp, struct phy_mem *phyp)
{
    struct mem_desc *mem_descs = machp->mem_descs;
    uint_t mem_count = machp->mem_desc_count;
    for (uint_t i = 1; i < mem_count; i++)
    {
        add_mem_to_slab(&phyp->slabs[BLKSZ_HEAD_MAX - 1], &mem_descs[i], ADD_EMPTY_FLAG);
    }
    return;
}

static void init_kernel_mem(struct mach_desc *machp, struct phy_mem *phyp)
{
    struct mem_desc *descs = machp->mem_descs;
    adr_t kernel_start = machp->kmem_start, kernel_end = machp->kmem_end;
    printf("kernel start:0x%x,end:0x%x\n", kernel_start, kernel_end);
    printf("descs[0]:0x%x\n", descs[0].start_addr);
    if ((kernel_end - kernel_start) > BLK128KB_SIZE || kernel_start < descs[0].start_addr)
    {
        sys_die("oh oh i am die!\n"); // 死机
    }
    u32_t count = 1;
    u32_t flag = MAP_FLAGES_VAL(0, MAPF_ACSZ_128KB, MAPF_SZ_4MB);
    descs[0].map_count = count;
    descs[0].flag = flag;

    add_mem_to_slab(&phyp->slabs[0], &descs[0], ADD_PARTIAL_FLAG);
    return;
}

static void init_phy_mem()
{
    phy_mem_init(&os_phy_mem);
    mem_list_init(&os_mach, &os_phy_mem);
    return;
}

void init_mm()
{
    init_mem_desc(&os_mach);
    init_phy_mem();

    // 下面这个函数要格外注意
    init_kernel_mem(&os_mach, &os_phy_mem);
    return;
}

static struct mem_slab *find_slab_by_size(struct mem_slab **ret_slab, size_t *ret_size, size_t size)
{
    struct phy_mem *phyp = &os_phy_mem;
    struct mem_slab *tmp = NULL;
    for (uint_t i = 0; i < BLKSZ_HEAD_MAX; i++)
    {
        if (phyp->slabs[i].size == size)
        {
            tmp = &phyp->slabs[i];
            *ret_slab = tmp;
            *ret_size = phyp->slabs[i].size;
            goto next_step;
        }
    }
    tmp = NULL;
    *ret_slab = NULL;
    *ret_size = 0;

next_step:
    if (tmp == NULL)
    {
        return NULL;
    }
    if ((list_empty(&tmp->empty_list) == FALSE) || (list_empty(&tmp->partial_list) == FALSE))
    {
        return tmp;
    }
    tmp = &phyp->slabs[BLKSZ_HEAD_MAX - 1];
    if (list_empty(&tmp->empty_list) == FALSE)
    {
        return tmp;
    }
    return NULL;
}

static struct mem_desc *find_desc_in_slab(struct mem_slab *slab)
{
    struct mem_desc *desc = NULL;

    // 先在非满队列中看有没有struct mem_desc结构，如果有就是第一个
    if (list_empty(&slab->partial_list) == FALSE)
    {
        desc = list_entry(slab->partial_list.next, struct mem_desc, list);
        return desc;
    }
    // 然后在空队列中看有没有struct mem_desc结构，如果有就是第一个
    if (list_empty(&slab->empty_list) == FALSE)
    {
        desc = list_entry(slab->empty_list.next, struct mem_desc, list);
        return desc;
    }
    // 都没有的话就返回NULL，不可能在full_list中，因为它里面装的是没有空闲内存块的
    // struct mem_desc结构
    return NULL;
}

static adr_t alloc_blk_on_desc(u32_t mflag, u32_t mask, u32_t bits, struct mem_slab *alloc_slab, struct mem_slab *moveto_slab)
{
    adr_t ret_adr = NULL;
    struct mem_desc *desc = NULL;
    uint_t bi = 0;
    u32_t flag = 0xffffff0f;
    if (bits > 32)
    {
        return NULL;
    }

    desc = find_desc_in_slab(alloc_slab);
    // 没有找到mmapdsc_t结构，当然不能继续执行了，只能返回NULL
    if (desc == NULL)
    {
        return NULL;
    }
    // 循环扫描map->map_allcount中的位到bitls位数结束，记录哪个位为0
    for (; bi < bits; bi++)
    {
        if (((desc->map_count >> bi) & 1) == 0)
        {
            goto next_step;
        }
    }
    // 如果循环结束后map->map_allcount中没有为0的位，则bi=0xffffffff
    bi = 0xffffffff;
next_step:
    if (bi == 0xffffffff)
    {
        return NULL;
    }
    // 计算返回地址，并且对计算出的地址进行合理化检杳
    ret_adr = desc->start_addr + (moveto_slab->size * bi);
    if (ret_adr < desc->start_addr || ret_adr >= desc->end_addr)
    {
        return NULL;
    }
    // 将map->map_allcount中的第bi位置为1，表示已分配
    desc->map_count |= (1 << bi);
    // 清除mmapdsc_t结构中的相关标志位
    desc->flag &= flag;
    // 设置新的标志位，该标志位正是表示其中小内存块大小的位
    desc->flag |= mflag;
    // 如果分配后的desc->map_count中的位和mask码相等，表示其中没有空闲内存块了，所以要移动到(满)队列中去
    if ((desc->map_count & mask) == mask)
    {
        list_move_tail(&desc->list, &moveto_slab->full_list);
        return ret_adr;
    }
    // 否则移动到mvtoaflp->afl_fuemlsth(非满)队列中去
    list_move_tail(&desc->list, &moveto_slab->partial_list);
    return ret_adr;
}

static adr_t alloc_blk_in_slab(struct mem_slab *alloc_slab, struct mem_slab *moveto_slab, size_t size)
{
    adr_t ret_adr = NULL;

    switch (size)
    {
    case BLK128KB_SIZE:
        ret_adr = alloc_blk_on_desc(MAPF_ACSZ_128KB, BLK128KB_MASK, BLK128KB_BITL, alloc_slab, moveto_slab);
        break;
    case BLK256KB_SIZE:
        ret_adr = alloc_blk_on_desc(MAPF_ACSZ_256KB, BLK256KB_MASK, BLK256KB_BITL, alloc_slab, moveto_slab);
        break;
    case BLK512KB_SIZE:
        ret_adr = alloc_blk_on_desc(MAPF_ACSZ_512KB, BLK512KB_MASK, BLK512KB_BITL, alloc_slab, moveto_slab);
        break;
    case BLK1MB_SIZE:
        ret_adr = alloc_blk_on_desc(MAPF_ACSZ_1MB, BLK1MB_MASK, BLK1MB_BITL, alloc_slab, moveto_slab);
        break;
    case BLK2MB_SIZE:
        ret_adr = alloc_blk_on_desc(MAPF_ACSZ_2MB, BLK2MB_MASK, BLK2MB_BITL, alloc_slab, moveto_slab);
        break;
    case BLK4MB_SIZE:
        ret_adr = alloc_blk_on_desc(MAPF_ACSZ_4MB, BLK4MB_MASK, BLK4MB_BITL, alloc_slab, moveto_slab);
        break;
    default:
        ret_adr = NULL;
        break;
    }
    return ret_adr;
}

static adr_t alloc_mem_blk_core(size_t blk_size)
{
    struct phy_mem *phyp = &os_phy_mem;
    size_t ret_size = 0;
    cpuflg_t flag;
    adr_t ret_addr = NULL;
    struct mem_slab *all_slab = NULL;
    struct mem_slab *tmp_slab = NULL;

    spin_lock_irqsave(&phyp->lock, &flag);

    tmp_slab = find_slab_by_size(&all_slab, &ret_size, blk_size);
    if (NULL == tmp_slab || NULL == all_slab || 0 == ret_size)
    {
        ret_addr = NULL;
        goto return_step;
    }
    if (tmp_slab->size != ret_size && tmp_slab->size != BLK4MB_SIZE)
    {
        ret_addr = NULL;
        goto return_step;
    }
    ret_addr = alloc_blk_in_slab(tmp_slab, all_slab, ret_size);

return_step:
    spin_unlock_irqrestore(&phyp->lock, &flag);
    return ret_addr;
}

adr_t alloc_mem_blk(size_t blk_size)
{
    if (blk_size < BLK128KB_SIZE || blk_size > BLK4MB_SIZE)
    {
        return NULL;
    }
    return alloc_mem_blk_core(blk_size);
}

static struct mem_desc *find_desc_for_free(adr_t addr, struct mem_slab *slab)
{
    struct mem_desc *tmp = NULL;

    if (list_empty(&slab->full_list) == FALSE)
    {
        list_for_each_entry(tmp, &slab->full_list, list)
        {
            if (tmp->start_addr <= addr && addr < tmp->end_addr)
            {
                return tmp;
            }
        }
    }

    if (list_empty(&slab->partial_list) == FALSE)
    {
        list_for_each_entry(tmp, &slab->partial_list, list)
        {
            if (tmp->start_addr <= addr && addr < tmp->end_addr)
            {
                return tmp;
            }
        }
    }
    return NULL;
}

static bool_t __free_blk_in_slab(adr_t addr, struct mem_slab *in_slab, struct mem_slab *mvto_slab, u32_t flag, u32_t mask)
{
    struct mem_desc *descp = NULL;
    uint_t bit_nr;
    u32_t fg = 0xffffff0f;

    // 根据释放内存空间块的地址和所在的mem_slab结构，查找mem_desc结构
    descp = find_desc_for_free(addr, in_slab);
    if (descp == NULL)
    {
        return FALSE;
    }
    // 根据内存空间块的释放地址计算该内存空间块，在map_count中对应的位
    bit_nr = (uint_t)(addr - descp->start_addr) / (uint_t)(in_slab->size);
    if (bit_nr > 31)
    {
        return FALSE;
    }
    // 检查一下计算出的map_count中的对应位是否为0，为0就表示出错了
    if (((descp->map_count >> bit_nr) & 1)!= 1)
    {
        return FALSE;
    }
    // 把释放内存空间块在map_allcount中对应的位清0，表示已经释放
    descp->map_count &= (~(1 << bit_nr));

    // 如果map_count中表示内存空间块状态相应的位都是0，即它已经是一整块空
    // 闲内存时，就设置相应的标志，然后移动mem_dsc_t结构到最后一个mem_slab结构中
    if ((descp->map_count & mask) == 0)
    {
        descp->flag &= fg;
        descp->flag |= flag;
        list_move_tail(&descp->list, &mvto_slab->empty_list);
    }
    return TRUE;
}

static bool_t free_blk_in_slab(adr_t addr, size_t blk_size, struct mem_slab *in_slab, struct mem_slab *mvto_slab)
{
    bool_t retstus = FALSE;

    switch (blk_size)
    {
    case BLK128KB_SIZE:
        retstus = __free_blk_in_slab(addr, in_slab, mvto_slab, MAPF_ACSZ_4MB, BLK128KB_MASK);
        break;
    case BLK256KB_SIZE:
        retstus = __free_blk_in_slab(addr, in_slab, mvto_slab, MAPF_ACSZ_4MB, BLK256KB_MASK);
        break;
    case BLK512KB_SIZE:
        retstus = __free_blk_in_slab(addr, in_slab, mvto_slab, MAPF_ACSZ_4MB, BLK512KB_MASK);
        break;
    case BLK1MB_SIZE:
        retstus = __free_blk_in_slab(addr, in_slab, mvto_slab, MAPF_ACSZ_4MB, BLK1MB_MASK);
        break;
    case BLK2MB_SIZE:
        retstus = __free_blk_in_slab(addr, in_slab, mvto_slab, MAPF_ACSZ_4MB, BLK2MB_MASK);
        break;
    case BLK4MB_SIZE:
        retstus = __free_blk_in_slab(addr, in_slab, mvto_slab, MAPF_ACSZ_4MB, BLK4MB_MASK);
        break;
    default:
        retstus = FALSE;
        break;
    }
    return retstus;
}

static bool free_mem_blk_core(adr_t addr, size_t blk_size)
{
    struct phy_mem *phyp = &os_phy_mem;
    bool_t ret = FALSE;
    cpuflg_t flag;
    struct mem_slab *tmp = NULL;
    struct mem_slab *mvto_slab = &phyp->slabs[BLKSZ_HEAD_MAX - 1];

    spin_lock_irqsave(&phyp->lock, &flag);

    for (uint_t i = 0; i < BLKSZ_HEAD_MAX; i++)
    {
        if (phyp->slabs[i].size == blk_size)
        {
            tmp = &phyp->slabs[i];
            break;
        }
    }
    if (NULL == tmp)
    {
        spin_unlock_irqrestore(&phyp->lock, &flag);
        return FALSE;
    }

    ret = free_blk_in_slab(addr, blk_size, tmp, mvto_slab);
    spin_unlock_irqrestore(&phyp->lock, &flag);
    return ret;
}

bool free_mem_blk(adr_t addr, size_t blk_size)
{
    if (addr == NULL || blk_size < BLK128KB_SIZE || blk_size > BLK4MB_SIZE)
    {
        return FALSE;
    }
    return free_mem_blk_core(addr, blk_size);
}