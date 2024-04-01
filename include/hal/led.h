#ifndef __BSP_LED_H
#define __BSP_LED_H

#include <hal/imx6ul/imx6ul.h>

#define LED0	0
#define ON      1
#define OFF     0

/* 函数声明 */
void led_init(void);
void led_switch(int led, int status);
#endif