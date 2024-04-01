#ifndef __MM_H
#define __MM_H

#include <types/basetype.h>

void init_mem_pool();
adr_t kmem_pool_alloc_page(size_t size);
bool_t kmem_pool_free_page(adr_t addr, size_t size);

#endif // __MM_H