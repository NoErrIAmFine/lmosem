#ifndef __CPU_CTRL_H
#define __CPU_CTRL_H

#include <types/basetype.h>

#define CPU_USR_MODE 0x10
#define CPU_FIQ_MODE 0x11
#define CPU_IRQ_MODE 0x12
#define CPU_SVE_MODE 0x13
#define CPU_ABT_MODE 0x17
#define CPU_UND_MODE 0x1b
#define CPU_SYS_MODE 0x1f

#define CFIQ 0x40
#define CIRQ 0x80
#define CIRQFIQ 0xc0

void sys_die(char_t* errmsg);

#endif // __CPU_CTRL_H