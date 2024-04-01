#include <hal/platform.h>
#include <hal/uart.h>
#include <hal/imx6ul/clk.h>
#include <hal/led.h>
#include <hal/delay.h>
#include <hal/mm.h>
#include <hal/mach.h>
#include <hal/interrupt.h>
#include <hal/global.h>

void test_blkmm();

void init_hal()
{
    imx6u_clkinit(); /* 初始化系统时钟 	*/
    clk_enable();    /* 使能所有的时钟 	*/
    led_init();
    delay_init();
    init_platform();
    // 闪一下灯，表示执行到此
    //  {
    //      led_switch(0,1);
    //      mdelay(2000);
    //      led_switch(0,0);
    //      mdelay(2000);
    //      led_switch(0,1);
    //  }
    init_uart();
    init_mach();
    init_mm();
    init_interrupt();
    // test_blkmm();
    puts("what the fuck code!\n");
    return;
}

void test_blkmm()
{
    adr_t retadra[14], retadr = 0, retadrold = 0, retadrend = 0;
    size_t ablksz = 0;
    bool_t retstus = FALSE;
    retadrold = alloc_mem_blk(BLK128KB_SIZE);
    if (retadrold == NULL)
    {
        sys_die("NOT MEMALLOCBLKold return NULL\n");
        
    }

    if (free_mem_blk(retadrold, BLK128KB_SIZE) == FALSE)
    {
        sys_die("NOT MEMFREEBLKold return FALSE\n");

    }
        
    for (uint_t bli = 0; bli < 6; bli++)
    {
        ablksz = BLK128KB_SIZE << bli;
        for (uint_t i = 0; i < 14; i++)
        {
            retadr = alloc_mem_blk(ablksz);
            if (retadr == NULL)
                sys_die("NOT MEMALLOCBLK; return NULL\n");
            printf("allocblksz : % x return adrr : % x \n\r", ablksz, retadr);
            retadra[i] = retadr;
        }
        for (uint_t j = 0; j < 14; j++)
        {
            retstus = free_mem_blk(retadra[j], ablksz);
            if (retstus == FALSE)
                sys_die("NOT MEMFREEBLK; return FALSE");
            printf("freeblksz : % x free adrr : % x \n\r", ablksz, retadra[j]);
        }
    }
    retadrend = alloc_mem_blk(BLK128KB_SIZE);
    if (retadrend == NULL)
        sys_die("NOT MEMALLOCBLKend; return NULL");
    if (retadrend == retadrold)
        printf("TEST BLKMM OK !!\n\r");
    return;
}