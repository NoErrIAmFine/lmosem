#ifndef __UART_H
#define __UART_H

#include <hal/imx6ul/imx6ul.h>

/* 函数声明 */
void init_uart(void);
void hal_uart_io_init(void);
void hal_uart_disable(UART_Type *base);
void hal_uart_enable(UART_Type *base);
void hal_uart_softreset(UART_Type *base);
void hal_uart_setbaudrate(UART_Type *base,unsigned int baudrate,unsigned int srcclock_hz);
void putc(unsigned char c);
void puts(char *str);
unsigned char getc(void);
void raise(int sig_nr);

#endif // __UART_H