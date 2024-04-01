.section .lmosem_hal.vector,"ax"
.code 32
.align 0
.extern hal_undefins
.extern hal_swi
.extern hal_prefabt
.extern hal_dataabt
.extern hal_bugdie
.extern hal_irq
.extern hal_frq
.extern _start
.global vector
vector:
        ldr pc,lmosem_restart_l
        ldr pc,lmosem_undefins_l
        ldr pc,lmosem_swi_l
        ldr pc,lmosem_prefabt_l
        ldr pc,lmosem_dataabt_l
        ldr pc,lmosem_bugdie_l
        ldr pc,lmosem_irq_l
        ldr pc,lmosem_frq_l @把hal_frq的地址装载进PC完成跳转，上面的代码也一样
.align 4
lmosem_restart_l:  .word _start @在init.S文件中
lmosem_undefins_l:  .word hal_undefins
lmosem_swi_l:      .word hal_swi
lmosem_prefabt_l:  .word hal_prefabt
lmosem_dataabt_l:  .word hal_dataabt
lmosem_bugdie_l:   .word hal_bugdie
lmosem_irq_l:      .word hal_irq
lmosem_frq_l:      .word hal_frq @由链接器填入实际的地址，上面的代码也一样
.end