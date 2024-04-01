#include <types/basetype.h>
#include <hal/init.h>
#include <kernel/init.h>
#include <stdio.h>

LKHEAD_T void hal_start()
{
    init_hal();
    init_kernel();
    return;
}