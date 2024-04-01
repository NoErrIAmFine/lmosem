#ifndef __BSP_DELAY_H
#define __BSP_DELAY_H

#include <hal/imx6ul/imx6ul.h>

/* 函数声明 */
void delay_init(void);
void udelay(unsigned int usdelay);
void mdelay(unsigned int msdelay);

#endif

