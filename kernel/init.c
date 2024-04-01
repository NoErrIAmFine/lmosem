#include <kernel/init.h>
#include <kernel/mm.h>

void init_kernel()
{
    init_mem_pool();
    return;
}