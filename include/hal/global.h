#ifndef __GLOBAL_T_H
#define __GLOBAL_T_H

#ifdef	HAL_GOBAL_HEAD
#undef	EXTERN
#define EXTERN
#endif

#include <hal/mm.h>
#include <hal/interrupt.h>

#define HAL_DEF_GLOBAL_VAR(vartype,varname) \
EXTERN  __attribute__((section(".data"))) vartype varname

extern struct mach_desc os_mach;
extern struct phy_mem os_phy_mem;
extern struct phy_addr_space mach_addr_space[ADDR_SPACE_NR];
extern struct irq_desc mach_interrupt[INT_NUMS];

#endif // __GLOBAL_T_H