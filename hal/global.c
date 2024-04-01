#define HALGOBAL_HEAD

#include <types/basetype.h>
#include <hal/global.h>
#include <hal/mach.h>


struct mach_desc os_mach;
struct phy_mem os_phy_mem;
struct phy_addr_space mach_addr_space[ADDR_SPACE_NR] = {
    {ADDR_SPACE_SDRAM, 0, 0x80000000, 0x9fffffff},
    {ADDR_SPACE_NOT, DEV_TYPE_NOT, 0, 0},
    {ADDR_SPACE_NOT, DEV_TYPE_NOT, 0, 0},
    {ADDR_SPACE_NOT, DEV_TYPE_NOT, 0, 0},
    {ADDR_SPACE_NOT, DEV_TYPE_NOT, 0, 0},
    {ADDR_SPACE_NOT, DEV_TYPE_NOT, 0, 0},
    {ADDR_SPACE_NOT, DEV_TYPE_NOT, 0, 0},
};
struct irq_desc mach_interrupt[INT_NUMS];