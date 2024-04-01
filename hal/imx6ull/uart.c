#include <hal/uart.h>

/*
 * @description : 初始化串口 1,波特率为 115200
 * @param : 无
 * @return : 无
 */
void init_uart(void)
{
    /* 1、初始化串口 IO */
    hal_uart_io_init();

    /* 2、初始化 UART1 */
    hal_uart_disable(UART1);   /* 先关闭 UART1 */
    hal_uart_softreset(UART1); /* 软件复位 UART1 */

    UART1->UCR1 = 0;           /* 先清除 UCR1 寄存器 */
    UART1->UCR1 &= ~(1 << 14); /* 关闭自动波特率检测 */

    /*
     * 设置 UART 的 UCR2 寄存器，设置字长，停止位，校验模式，关闭硬件流控
     * bit14: 1 忽略 RTS 引脚
     * bit8: 0 关闭奇偶校验
     * bit6: 0 1 位停止位
     * bit5: 1 8 位数据位
     * bit2: 1 打开发送
     * bit1: 1 打开接收
     */
    UART1->UCR2 |= (1 << 14) | (1 << 5) | (1 << 2) | (1 << 1);
    UART1->UCR3 |= 1 << 2; /* UCR3 的 bit2 必须为 1 */

    /*
     * 设置波特率
     * 波特率计算公式:Baud Rate = Ref Freq / (16 * (UBMR + 1)/(UBIR+1))
     * 如果要设置波特率为 115200，那么可以使用如下参数:
     * Ref Freq = 80M 也就是寄存器 UFCR 的 bit9:7=101, 表示 1 分频
     * UBMR = 3124
     * UBIR = 71
     * 因此波特率= 80000000/(16 * (3124+1)/(71+1))
     * = 80000000/(16 * 3125/72)
     * = (80000000*72) / (16*3125)
     * = 115200
     */
    UART1->UFCR = 5 << 7; /* ref freq 等于 ipg_clk/1=80Mhz */
    UART1->UBIR = 71;
    UART1->UBMR = 3124;

#if 0
    uart_setbaudrate(UART1, 115200, 80000000); /* 设置波特率 */
#endif

    hal_uart_enable(UART1); /* 使能串口 */
}

/*
 * @description : 初始化串口 1 所使用的 IO 引脚
 * @param : 无
 * @return : 无
 */
void hal_uart_io_init(void)
{
    // 设置引脚复用
    IOMUXC_SetPinMux(IOMUXC_UART1_TX_DATA_UART1_TX, 0);
    IOMUXC_SetPinMux(IOMUXC_UART1_RX_DATA_UART1_RX, 0);

    // 设置引脚配置，比如上下拉，驱动能力这些
    IOMUXC_SetPinConfig(IOMUXC_UART1_TX_DATA_UART1_TX, 0x10B0);
    IOMUXC_SetPinConfig(IOMUXC_UART1_RX_DATA_UART1_RX, 0x10B0);
}

void hal_uart_disable(UART_Type *base)
{
    base->UCR1 &= ~(1 << 0);
}

void hal_uart_enable(UART_Type *base)
{
    base->UCR1 |= (1 << 0);
}
void hal_uart_softreset(UART_Type *base)
{
    base->UCR2 &= ~(1 << 0); /* 复位 UART */
    while ((base->UCR2 & 0x1) == 0)
        ; /* 等待复位完成 */
}

/*
 * @description : 波特率计算公式，
 * 可以用此函数计算出指定串口对应的 UFCR，
 * UBIR 和 UBMR 这三个寄存器的值
 * @param - base : 要计算的串口。
 * @param - baudrate : 要使用的波特率。
 * @param - srcclock_hz : 串口时钟源频率，单位 Hz
 * @return : 无
 */
void hal_uart_setbaudrate(UART_Type *base, unsigned int baudrate, unsigned int srcclock_hz)
{
    // uint32_t numerator = 0u;
    // uint32_t denominator = 0U;
    // uint32_t divisor = 0U;
    // uint32_t refFreqDiv = 0U;
    // uint32_t divider = 1U;
    // uint64_t baudDiff = 0U;
    // uint64_t tempNumerator = 0U;
    // uint32_t tempDenominator = 0u;

    // /* get the approximately maximum divisor */
    // numerator = srcclock_hz;
    // denominator = baudrate << 4;
    // divisor = 1;

    // while (denominator != 0)
    // {
    //     divisor = denominator;
    //     denominator = numerator % denominator;
    //     numerator = divisor;
    // }

    // numerator = srcclock_hz / divisor;
    // denominator = (baudrate << 4) / divisor;

    // /* numerator ranges from 1 ~ 7 * 64k */
    // /* denominator ranges from 1 ~ 64k */
    // if ((numerator > (UART_UBIR_INC_MASK * 7)) || (denominator > UART_UBIR_INC_MASK))
    // {
    //     uint32_t m = (numerator - 1) / (UART_UBIR_INC_MASK * 7) + 1;
    //     uint32_t n = (denominator - 1) / UART_UBIR_INC_MASK + 1;
    //     uint32_t max = m > n ? m : n;
    //     numerator /= max;
    //     denominator /= max;
    //     if (0 == numerator)
    //     {
    //         numerator = 1;
    //     }
    //     if (0 == denominator)
    //     {
    //         denominator = 1;
    //     }
    // }
    // divider = (numerator - 1) / UART_UBIR_INC_MASK + 1;

    // switch (divider)
    // {
    // case 1:
    //     refFreqDiv = 0x05;
    //     break;
    // case 2:
    //     refFreqDiv = 0x04;
    //     break;
    // case 3:
    //     refFreqDiv = 0x03;
    //     break;
    // case 4:
    //     refFreqDiv = 0x02;
    //     break;
    // case 5:
    //     refFreqDiv = 0x01;
    //     break;
    // case 6:
    //     refFreqDiv = 0x00;
    //     break;
    // case 7:
    //     refFreqDiv = 0x06;
    //     break;
    // default:
    //     refFreqDiv = 0x05;
    //     break;
    // }
    // /* Compare the difference between baudRate_Bps and calculated
    // * baud rate. Baud Rate = Ref Freq / (16 * (UBMR + 1)/(UBIR+1)).
    // * baudDiff = (srcClock_Hz/divider)/( 16 * ((numerator /
    // divider)/ denominator).
    // */
    // tempNumerator = srcclock_hz;
    // tempDenominator = (numerator << 4);
    // divisor = 1;
    // /* get the approximately maximum divisor */
    // while (tempDenominator != 0)
    // {
    //     divisor = tempDenominator;
    //     tempDenominator = tempNumerator % tempDenominator;
    //     tempNumerator = divisor;
    // }
    // tempNumerator = srcclock_hz / divisor;
    // tempDenominator = (numerator << 4) / divisor;
    // baudDiff = (tempNumerator * denominator) / tempDenominator;
    // baudDiff = (baudDiff >= baudrate) ? (baudDiff - baudrate) : (baudrate - baudDiff);

    // if (baudDiff < (baudrate / 100) * 3)
    // {
    //     base->UFCR &= ~UART_UFCR_RFDIV_MASK;
    //     base->UFCR |= UART_UFCR_RFDIV(refFreqDiv);
    //     base->UBIR = UART_UBIR_INC(denominator - 1);
    //     base->UBMR = UART_UBMR_MOD(numerator / divider - 1);
    // }
}

void putc(unsigned char c)
{
    while (((UART1->USR2 >> 3) & 0X01) == 0)
        ;                   /* 等待上一次发送完成 */
    UART1->UTXD = c & 0XFF; /* 发送数据 */
    /* 如果是/n换行符，则额外再发送一个/t */
    if(c == '\n')
        putc('\r');
}

void puts(char *str)
{
    char *p = str;

    while (*p)
        putc(*p++);
}

unsigned char getc(void)
{
    while ((UART1->USR2 & 0x1) == 0)
        ;               /* 等待接收完成 */
    return UART1->URXD; /* 返回接收到的数据 */
}

void raise(int sig_nr)
{

}