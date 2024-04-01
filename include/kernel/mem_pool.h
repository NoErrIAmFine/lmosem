#ifndef __MEM_POOL_H
#define __MEM_POOL_H

#include <types/basetype.h>
#include <types/list.h>
#include <types/spinlock_t.h>

#define MPALCLST_MAX 5

#define PMPLMAP_MAX 32
#define KMEMPALCSZ_MIN 1
#define KMEMPALCSZ_MAX 0x400000

#define OBJSORPAGE 2048
#define KPMPORHALM (PAGE_SIZE * 31)

#define MPLHTY_PAGE 1
#define MPLHTY_OBJS 2

#define OBJS_ALIGN(x) ALIGN(x, 4)
#define PHYMSA_MAX 512
#define PAGE_SIZE 0x1000

struct mem_pool_map
{
    adr_t start;               // 保存内存池中一个页面的首地址
    struct mem_pool_map *next; // 指向自身的一个指针
};

struct mem_pool_head
{
    spinlock_t lock;       // 保护其自身的自旋锁
    struct list_head list; // 链表
    uint_t type;           // 内存池管理头的类型
    adr_t start;           // 内存池的开始地址
    adr_t end;             // 内存池的结束地址
    adr_t firtfreadr;
    struct mem_pool_map *next_map; // 指向第一个空闲页(即下一个将分配的页)状态描述结构的指针
    uint_t obj_count;              // 池中对象（页）的个数
    uint_t alloc_size;
    uint_t obj_size; // 池中对象（页）的大小
    uint_t nxtpsz;
    uint_t afindx;                 // 已经分配对象（页）的个数
    uint_t pool_map_count;         // 页状态描述结构的个数
    struct mem_pool_map *pool_map; // 页状态描述结构的存放地址
};

struct kmem_pool
{
    spinlock_t lock;                       // 保护其自身的自旋锁
    struct list_head list;                 // 链表
    uint_t status;                         // 状态
    uint_t flag;                           // 标志
    spinlock_t page_lock;                  // 保护页级内存池的自旋锁
    spinlock_t word_lock;                  // 保护字级内存池的自旋锁
    uint_t page_pool_count;                // 页级内存池的个数
    uint_t word_pool_count;                // 字级内存池的个数
    struct list_head page_pool_head;       // 挂载页级内存池的链表头
    struct list_head word_pool_head;       // 挂载字级内存池的链表头
    struct mem_pool_head *page_pool_cache; // 保存当前操作过的页级mem_pool_head结构
    struct mem_pool_head *word_pool_cache; // 保存当前操作过的字级mplhead_t结构
};

#endif // __PAGE_MEM_H