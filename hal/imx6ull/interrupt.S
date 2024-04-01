.section .lmosem_hal.intvect,"ax"
.code 32
.align 4
.global hal_undefins
.global hal_swi
.global hal_prefabt
.global hal_dataabt
.global hal_bugdie
.global hal_irq
.global hal_frq
hal_undefins:
bl _hal_undefins
hal_swi:
bl _hal_swi
hal_prefabt:
bl _hal_prefabt
hal_dataabt:
bl _hal_dataabt
hal_bugdie:
bl _hal_bugdie
hal_irq:
bl _hal_irq
hal_frq:
bl _hal_frq