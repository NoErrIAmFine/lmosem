#include <hal/imx6ul/cpu_ctrl.h>
#include <hal/uart.h>

void sys_die(char_t* err_msg)
{
    puts(err_msg);
    // printfk("LMOSEM SYSTEM IS DIE %s",errmsg);
    for(;;);
    return;
}