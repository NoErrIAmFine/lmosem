#include <kernel/global.h>
#include <kernel/mem_pool.h>
#include <hal/imx6ul/cpu_ctrl.h>
#include <hal/mm.h>
#include <stdio.h>

static void mem_pool_head_init(struct mem_pool_head *initp)
{
    // 初始化自旋锁
    spin_lock_init(&initp->lock);
    // 初始化链表
    INIT_LIST_HEAD(&initp->list);
    initp->type = 0;
    initp->start = 0;
    initp->end = 0;
    initp->firtfreadr = NULL;
    initp->next_map = NULL;
    initp->obj_count = 0;
    initp->alloc_size = 0;
    initp->obj_size = 0;
    initp->nxtpsz = 0;
    initp->afindx = 0;
    initp->pool_map_count = 0;
    // 可以看出mem_pool_map结构的开始地址正是紧跟mem_pool_head结构其后的
    initp->pool_map = (struct mem_pool_map *)(((uint_t)initp) + sizeof(struct mem_pool_head));
    return;
}

static struct mem_pool_head *page_mpool_init(struct kmem_pool *kpoolp, struct mem_pool_head *initp, size_t size, adr_t start, adr_t end)
{
    if (((start & 0xfff) != 0) || ((end - start) < ((PAGE_SIZE * 2) - 1)))
        return NULL;
    // 因为内存空间块的第一页要存放mplhead_t、struct mem_pool_map数据结构
    adr_t sadr = start + PAGE_SIZE;
    uint_t pi = 0, pnr = 0;
    // 和其他数据结构初始化函数一样，是把数据结构本身初始化为默认状态
    mem_pool_head_init(initp);
    // 把这个内存池的类型标记为页级内存池类型
    initp->type = MPLHTY_PAGE;
    // 设置内存块(池)的开始地址
    initp->start = start;
    // 设置内存块(池)的结束地址
    initp->end = end;
    // 设置内存池实际分配对象的大小
    initp->alloc_size = size;
    // 内存池对象的大小，对于页级内存池，它和aliobsz的值相同
    initp->obj_size = size;
    // 每循环一次，就初始化一个struct mem_pool_map数据结构
    for (;;)
    {
        // 如果(sadr+size-1)大于end，则跳出循环
        if ((sadr + size - 1) > end)
            break;
        initp->pool_map[pi].start = sadr;
        initp->pool_map[pi].next = &(initp->pool_map[pi + 1]);
        // sadr=sadr+msize循环一次就加上一个分配大小
        sadr += size;
        pi++;
    }
    if (pi > 0)
    {
        // 设置最后一个struct mem_pool_map结构的pgl_next_map域为NULL
        initp->pool_map[pi - 1].next = NULL;
        pnr = pi;
        // 设置next_map指向第一个struct mem_pool_map结构
        initp->next_map = &(initp->pool_map[0]);
        goto add_step;
    }
    initp->pool_map[pi].next = NULL;
    initp->pool_map[pi].start = NULL;
    pnr = pi;

add_step:
    // 一共有几个页面对象
    initp->obj_count = pnr;
    // 一共有几个struct mem_pool_map数据结构
    initp->pool_map_count = pnr;
    // 把新内存池(mplhead_t)加入到kmempool_t结构的链表中
    list_add_tail(&initp->list, &kpoolp->page_pool_head);
    // 表示又建立了一个页面内存池
    kpoolp->page_pool_count++;
    return initp;
}

#define NEW_PAGE_MEM_POOL(_size) {              \
    blk_addr = alloc_mem_blk(_size);            \
    if (blk_addr == NULL)                       \
        return NULL;                            \
    headp = page_mpool_init(kpoolp, (struct mem_pool_head *)blk_addr, size, blk_addr, (blk_addr + _size - 1));                              \
    if (headp == NULL)                          \
        sys_die("new pg mpool err 1");          \
    return headp;                               \
}

static struct mem_pool_head *new_page_mpool(struct kmem_pool *kpoolp, size_t size)
{
    struct mem_pool_head *headp = NULL;
    size_t page_nr = size >> 12;
    adr_t blk_addr = NULL;
    if (page_nr < 1)
        return NULL;
    
    if (page_nr <= 2)
    {
        NEW_PAGE_MEM_POOL(BLK128KB_SIZE);
    }
    if (page_nr <= 4)
    {
        NEW_PAGE_MEM_POOL(BLK256KB_SIZE);
    }
    if (page_nr <= 8)
    {
        NEW_PAGE_MEM_POOL(BLK256KB_SIZE);
    }
    if (page_nr <= 16)
    {
        NEW_PAGE_MEM_POOL(BLK256KB_SIZE);
    }
    if (page_nr <= 31)
    {
        NEW_PAGE_MEM_POOL(BLK128KB_SIZE);
    }
    return NULL;
}

static struct mem_pool_head *pool_head_is_ok(struct mem_pool_head *headp, size_t size)
{
    // next_map是否为NULL，为NULL即表示页面已经分配完了
    if (headp->next_map == NULL)
        return NULL;
    // 是否为页面内存池
    if (headp->type != MPLHTY_PAGE)
        return NULL;
    // 实际对象(页面)大小是否与请求大小相等
    if (headp->alloc_size != size)
        return NULL;
    // 分配计数是否大于等于对象(页面)个数
    if (headp->afindx >= headp->obj_count)
        return NULL;
    // 以上测试通过就返回mhp，表示一个页面内存池是合乎分配要求的
    return headp;
}

static struct mem_pool_head *find_pool_head_by_size(struct kmem_pool *kpoolp, size_t size)
{
    struct mem_pool_head *ret_head;
    struct mem_pool_head *headp;
    
    if (kpoolp->page_pool_cache != NULL)
    {
        ret_head = kpoolp->page_pool_cache;
        ret_head = pool_head_is_ok(ret_head, size);
        if (ret_head != NULL)
        {
            return ret_head;
        }
    }
    
    list_for_each_entry(headp, &kpoolp->page_pool_head, list)
    {
        ret_head = pool_head_is_ok(headp, size);
        if (ret_head != NULL)
        {
            kpoolp->page_pool_cache = ret_head;
            return ret_head;
        }
    }
    
    return NULL;
}

static adr_t page_new_on_mplhead(struct mem_pool_head *headp)
{
    struct mem_pool_map *map;
    if (headp->afindx >= headp->obj_count)
        return NULL;
    if (headp->next_map != NULL)
    {
        // 把当前空闲的struct mem_pool_map结构保存到map中
        map = headp->next_map;
        // 指向下一个空闲的struct mem_pool_map结构
        headp->next_map = map->next;
        // 把当前空闲struct mem_pool_map结构的pgl_next_map域置为NULL，以断掉链接
        map->next = NULL;
        // 增加分配计数
        headp->afindx++;
        // 返回当前空闲struct mem_pool_map结构中pgl_start域的值，这正是空闲页面的首地址
        return map->start;
    }
    return NULL;
}

adr_t kmem_pool_alloc_page(size_t size)
{
    cpuflg_t flag;
    struct mem_pool_head *headp;
    adr_t ret_addr = NULL;
    struct kmem_pool *kpoolp = &os_kmem_pool;
    spin_lock_irqsave(&kpoolp->page_lock, &flag);
    
    headp = find_pool_head_by_size(kpoolp,size);
    
    if(NULL == headp){
        headp = new_page_mpool(kpoolp,size);
        
        if(NULL == headp){
            ret_addr = NULL;
            goto return_step;
        }
    }
    
    ret_addr = page_new_on_mplhead(headp);
    
return_step:
    spin_unlock_irqrestore(&kpoolp->page_lock, &flag);
    return ret_addr;
}

static bool_t del_page_mpool(struct kmem_pool *kpoolp, struct mem_pool_head *headp)
{
    // 内存池非空闲，所以不能继续删除这个内存池了
    if (headp->afindx > 0)
    {
        return TRUE;
    }
    // 计算内存池所占用的这块内存空间的大小
    size_t frsz = headp->end - headp->start + 1;
    // 把这个内存池的开始地址，作为释放地址
    adr_t fradr = headp->start;
    // 把这个内存池从kmempool_t结构中删除，即"断链"
    list_del(&headp->list);
    // 检查页级内存池的个数，如果小于1，那么内核代码肯定出了大问题，只能死机
    if (kpoolp->page_pool_count < 1)
    {
        sys_die("del_page_mpool kpoolp->page_pool_count < 1");
    }
    // 减少页级内存池的个数，因为已经要删除一个了
    kpoolp->page_pool_count--;
    if (kpoolp->page_pool_cache == headp)
    {
        kpoolp->page_pool_cache = NULL;
    }
    // 如果块级内存释放失败，也只能死机
    if (free_mem_blk(fradr, frsz) == FALSE)
    {
        sys_die("del_page_mpool free_mem_blk err");
    }
    return TRUE;
}

static bool_t delete_page_on_mplhead(struct mem_pool_head *headp, adr_t addr)
{
    struct mem_pool_map *map;
    // 进行例行检查，mh_afindx<1表示没有分配过，所以更谈不上释放了
    // mh_pmnr<1则表示没有 struct mem_pool_map 结构
    if (headp->afindx < 1 || headp->pool_map_count < 1)
    {
        return FALSE;
    }
    // 搜索所有的 struct mem_pool_map结构，看看哪个 struct mem_pool_map结构的start域中的值
    // 等于释放地址，同时让其pgl_next域等于NULL
    // 然后把这个 struct mem_pool_map 结构的指针保存在map中
    for (uint_t i = 0; i < headp->pool_map_count; i++)
    {
        if (addr == headp->pool_map[i].start)
        {
            map = &headp->pool_map[i];
            // 已经被分配的map其next都为null，所有空闲的map才会通过next串在一起
            if (map->next != NULL)
            {
                return FALSE;
            }
            goto del_step;
        }
    }
    // 如果没找到这个pglmap_t结构，返回错误
    return FALSE;
del_step:
    // 把 map 放到空闲链中
    map->next = headp->next_map;
    headp->next_map = map;
    // 减少分配计数，表示又释放了一个对象(内存页面)
    headp->afindx--;
    return TRUE;
}

static struct mem_pool_head *deleted_pool_head_isok(struct mem_pool_head *headp, adr_t addr, size_t size)
{
    // 是否为页级内存池
    if (headp->type != MPLHTY_PAGE)
        return NULL;
    // 实际对象大小是否和页面释放大小相同
    if (headp->alloc_size != size)
        return NULL;
    // 内存池中的页面是否已经分配过了，为0即表示没分配过，没有分配当然不能释放
    if (headp->afindx == 0)
        return NULL;
    // 释放地址是不是落在这个内存池中了
    if (addr < (headp->start + PAGE_SIZE) || (addr + size - 1) > headp->end)
        return NULL;
    // 以上都判断无误后就返回这个mplhead_t结构的指针
    return headp;
}

static struct mem_pool_head *find_deleted_pool_head(struct kmem_pool *kpoolp, adr_t addr, size_t size)
{
    struct mem_pool_head *ret_head;

    if (kpoolp->page_pool_cache != NULL)
    {
        ret_head = kpoolp->page_pool_cache;
        ret_head = deleted_pool_head_isok(ret_head, addr, size);
        if (ret_head != NULL)
        {
            return ret_head;
        }
    }
    list_for_each_entry(ret_head, &kpoolp->page_pool_head, list)
    {
        ret_head = deleted_pool_head_isok(ret_head, addr, size);
        if (ret_head != NULL)
        {
            return ret_head;
        }
    }
    return NULL;
}

bool_t kmem_pool_free_page(adr_t addr, size_t size)
{
    cpuflg_t flag;
    struct mem_pool_head *headp;
    bool_t ret = FALSE;
    struct kmem_pool *kpoolp= &os_kmem_pool;

    spin_lock_irqsave(&kpoolp->lock,&flag);

    //查找对应的mem_pool_head
    headp = find_deleted_pool_head(kpoolp,addr,size);
    if(NULL == headp){
        ret = FALSE;
        goto return_step;
    }
    //释放内存页面
    if(FALSE == delete_page_on_mplhead(headp,addr)){
        ret = FALSE;
        goto return_step;
    }
    //尝试释放块级内存
    if(FALSE == del_page_mpool(kpoolp,headp)){
        ret = FALSE;
        goto return_step;
    }
    ret = TRUE;
return_step:
    spin_unlock_irqrestore(&kpoolp->lock,&flag);
    return ret;
}
static void kmem_pool_init(struct kmem_pool *poolp)
{
    spin_lock_init(&poolp->lock);
    INIT_LIST_HEAD(&poolp->list);
    poolp->status = 0;
    poolp->flag = 0;
    spin_lock_init(&poolp->page_lock);
    spin_lock_init(&poolp->word_lock);
    poolp->page_pool_count = 0;
    poolp->word_pool_count = 0;
    INIT_LIST_HEAD(&poolp->page_pool_head);
    INIT_LIST_HEAD(&poolp->word_pool_head);
    poolp->page_pool_cache = NULL;
    poolp->word_pool_cache = NULL;
    return;
}

typedef struct s_adrsz
{
    adr_t adr;//存放分配内存空间的地址
    size_t sz; //存放分配内存空间的大小
}adrsz_t;

//测试函数
void testpgmpool()
{
    adrsz_t adsz[10];
    size_t alcsz = 0x1000;
    
    adsz[0].sz = alcsz;
    adsz[0].adr = kmem_pool_alloc_page(alcsz);
    adsz[1].sz = alcsz;
    adsz[1].adr = kmem_pool_alloc_page(alcsz);
    alcsz = 0x1500;
    
    adsz[2].sz = alcsz;
    adsz[2].adr = kmem_pool_alloc_page(alcsz);
    adsz[3].sz = alcsz;
    adsz[3].adr = kmem_pool_alloc_page(alcsz);
    alcsz = 0x3000;
    adsz[4].sz = alcsz;
    adsz[4].adr = kmem_pool_alloc_page(alcsz);
    adsz[5].sz = alcsz;
    adsz[5].adr = kmem_pool_alloc_page(alcsz);
    alcsz = 0x3200;
    adsz[6].sz = alcsz;
    adsz[6].adr = kmem_pool_alloc_page(alcsz);
    adsz[7].sz = alcsz;
    adsz[7].adr = kmem_pool_alloc_page(alcsz);
    alcsz = 0x7000;
    adsz[8].sz = alcsz;
    adsz[8].adr = kmem_pool_alloc_page(alcsz);
    adsz[9].sz = alcsz;
    adsz[9].adr = kmem_pool_alloc_page(alcsz);
    for (int i = 0; i < 10; i++)
    {
        printf("adsz[%x] sz:%x adr:%x\n\r", i, adsz[i].sz, adsz[i].adr);
    }
    struct mem_pool_head *retmhp;

    list_for_each_entry(retmhp, &os_kmem_pool.page_pool_head, list)
    {
        printf("mph_t_adr:%x mph_t.mh_end:%x mph_t.mh_objsz:%x mph_t.mh_objnr:%x\n\r", retmhp, retmhp->end, retmhp->obj_size, retmhp->obj_count);
    }
    return;
}

//测试函数
void cmp_adrsz(adrsz_t *assp, uint_t nr)
{
    for (uint_t i = 0; i < nr; i++)
    {
        for (uint_t j = 0; j < nr; j++)
        {
            if (i != j)
            {
                // 比较每个分配地址是否和其他分配地址相等
                if (assp[i].adr == assp[j].adr)
                {
                    sys_die("cmp adr start err");
                }
            }
        }
    }
    for (uint_t k = 0; k < nr; k++)
    {
        for (uint_t h = 0; h < nr; h++)
        {
            if (k != h)
            {
                // 比较每个地址区间是否和其他地址区间相等
                if ((assp[k].adr + assp[k].sz) == (assp[h].adr + assp[h].sz))
                {
                    sys_die("cmp adr end err");
                }
            }
        }
    }
    for (uint_t l = 0; l < nr; l++)
    {
        for (uint_t m = 0; m < nr; m++)
        {
            if (l != m)
            {
                // 比较每个地址区间是否和其他地址区间相交
                if ((assp[l].adr >= (assp[m].adr)) && ((assp[l].adr + assp[l].sz) <= (assp[m].adr + assp[m].sz)))
                {
                    sys_die("cmp adr in err");
                }
            }
        }
    }
    return;
}

void testpagemgr()
{
    adrsz_t adsz[30];
    size_t alcsz = 0x1000;
    for (; alcsz < 0x19000; alcsz += 0x1000)
    {
        // 同一种分配大小，分配30次
        for (int i = 0; i < 30; i++)
        {
            adsz[i].sz = alcsz;
            adsz[i].adr = kmem_pool_alloc_page(alcsz);
            if (adsz[i].adr == NULL)
            {
                sys_die("testpagemgr kmempool_new err");
            }
            printf("adsz[%x] sz:%x adr:%x\n\r", i, adsz[i].sz, adsz[i].adr);
        }
        // 比较adsz中的分配地址是否正确
        cmp_adrsz(adsz, 30);
        // 依次释放所有已经分配的页面
        for (int j = 0; j < 30; j++)
        {
            if (kmem_pool_free_page(adsz[j].adr, adsz[j].sz) == FALSE)
            {
                sys_die("testpagemgr kmempool_delete err");
            }
            printf("delete adsz[%x] sz:%x adr:%x\n\r", j, adsz[j].sz, adsz[j].adr);
        }
    }
    printf("oskmempool.mp_pgmplnr:%x\n\r", os_kmem_pool.page_pool_count);
    return;
}

void init_mem_pool()
{
    kmem_pool_init(&os_kmem_pool);
    // testpgmpool();
    // testpagemgr();
    return;
}
