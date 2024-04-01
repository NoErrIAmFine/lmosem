#ifndef __MM_H
#define __MM_H

#include <types/basetype.h>
#include <types/list.h>
#include <types/spinlock_t.h>

#define BLKSZ_HEAD_MAX 6

#define MAPF_SZ_BIT 0
#define MAPF_ACSZ_BIT 4
#define MAPF_SZ_16KB (1<<MAPF_SZ_BIT)
#define MAPF_SZ_32KB (2<<MAPF_SZ_BIT)
#define MAPF_SZ_4MB (8<<MAPF_SZ_BIT)

#define MAPF_ACSZ_128KB (1<<MAPF_ACSZ_BIT)
#define MAPF_ACSZ_256KB (2<<MAPF_ACSZ_BIT)
#define MAPF_ACSZ_512KB (3<<MAPF_ACSZ_BIT)
#define MAPF_ACSZ_1MB (4<<MAPF_ACSZ_BIT)
#define MAPF_ACSZ_2MB (5<<MAPF_ACSZ_BIT)
#define MAPF_ACSZ_4MB (6<<MAPF_ACSZ_BIT)

#define BLK128KB_SIZE (0x20000)
#define BLK256KB_SIZE (0x40000)
#define BLK512KB_SIZE (0x80000)
#define BLK1MB_SIZE (0x100000)
#define BLK2MB_SIZE (0x200000)
#define BLK4MB_SIZE (0x400000)

#define BLK128KB_BITL (32)
#define BLK256KB_BITL (16)
#define BLK512KB_BITL (8)
#define BLK1MB_BITL (4)
#define BLK2MB_BITL (2)
#define BLK4MB_BITL (1)

#define BLK128KB_MASK (0xffffffff)
#define BLK256KB_MASK (0xffff)
#define BLK512KB_MASK (0xff)
#define BLK1MB_MASK (0xf)
#define BLK2MB_MASK (0x3)
#define BLK4MB_MASK (0x1)

#define ADD_EMPTY_FLAG      1
#define ADD_FULL_FLAG       2
#define ADD_PARTIAL_FLAG    3

#define MAPONE_SIZE (0x400000)
#define MAP_FLAGES_VAL(RV,SALLOCSZ,MSZ) (RV|SALLOCSZ|MSZ)

#define DEV_TYPE_NOT 0xffffffff
enum addr_space_type{
    ADDR_SPACE_NOT,
    ADDR_SPACE_IO,
    ADDR_SPACE_SDRAM,
    ADDR_SPACE_RAM,
    ADDR_SPACE_ROM,
    ADDR_SPACE_NORFLASH,
    ADDR_SPACE_NANDFLASH,
    ADDR_SPACE_NR
};

struct phy_addr_space
{
    u32_t flag;         //地址空间的类型如ADRSPCE_IO、ADRSPCE_SDRAM等
    u32_t dev_type;     //如果地址空间的类型是ADRSPCE_IO，则指示其设备类型。这里不必关心，这是为后面扩展所用的
    adr_t addr_start;   //地址空间的开始地址
    adr_t addr_end;     //地址空间的结束地址
};

/*它管理4MB大小的内存块，在实际分配时，所描述的4MB
 *可以被分割成128KB、256KB、512KB1MB、2MB、4MB，只能
 *分成同一大小的等份*/
struct mem_desc
{
    struct list_head    list;
    spinlock_t          lock;
    adr_t               start_addr;
    adr_t               end_addr;
    u32_t               map_count;
    u32_t               flag;
};

/*用于挂载同一种内存块大小的mmapdsc_t结构队列，如分割成128KB大小的
 *并对其分类，分配完的、分配一部分的、未分配的*/
struct mem_slab
{
    spinlock_t  lock;               //保护其自身的自旋锁
    size_t      size;               //表示该类的大小
    struct list_head full_list;     //同类已经分配完的mmapdsc_t结构链表头
    struct list_head empty_list;    //同类全部未分配的（空闲的）mmapdsc_t结构链表头
    struct list_head partial_list;  //同类已经分配一部分的mmapdsc_t结构链表头
    
};

struct phy_mem
{ 
    struct list_head list;
    spinlock_t lock;
    uint_t free_blks;
    uint_t alloc_blks;
    struct mem_slab slabs[BLKSZ_HEAD_MAX];
    /*形成了128KB~4MB大小的alcfrelst_t队列数组 [0]128KB
     *[1]256KB [2]512KB [3]1MB [4]2MB [5]4MB*/
};

void init_mm();
adr_t alloc_mem_blk(size_t blk_size);
bool free_mem_blk(adr_t addr, size_t blk_size);

#endif // __MM_H