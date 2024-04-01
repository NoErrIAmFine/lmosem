#include <hal/delay.h>

/*
 * @description	: 延时有关硬件初始化,主要是GPT定时器
				  GPT定时器时钟源选择ipg_clk=66Mhz
 * @param		: 无
 * @return 		: 无
 */
void delay_init(void)
{
	GPT1->CR = 0; 					/* 清零，bit0也为0，即停止GPT  			*/

	GPT1->CR = 1 << 15;				/* bit15置1进入软复位 				*/
	while((GPT1->CR >> 15) & 0x01);	/*等待复位完成 						*/
	
	/*
   	 * GPT的CR寄存器,GPT通用设置
   	 * bit22:20	000 输出比较1的输出功能关闭，也就是对应的引脚没反应
     * bit9:    0   Restart模式,当CNT等于OCR1的时候就产生中断
     * bit8:6   001 GPT时钟源选择ipg_clk=66Mhz
     * bit
  	 */
	GPT1->CR = (1<<6);

	/*
     * GPT的PR寄存器，GPT的分频设置
     * bit11:0  设置分频值，设置为0表示1分频，
     *          以此类推，最大可以设置为0XFFF，也就是最大4096分频
	 */
	GPT1->PR = 65;	/* 设置为65，即66分频，因此GPT1时钟为66M/(65+1)=1MHz */

	 /*
      * GPT的OCR1寄存器，GPT的输出比较1比较计数值，
      *	GPT的时钟为1Mz，那么计数器每计一个值就是就是1us。
      * 为了实现较大的计数，我们将比较值设置为最大的0XFFFFFFFF,
      * 这样一次计满就是：0XFFFFFFFFus = 4294967296us = 4295s = 71.5min
      * 也就是说一次计满最多71.5分钟，存在溢出
	  */
	GPT1->OCR[0] = 0XFFFFFFFF;

	GPT1->CR |= 1<<0;			//使能GPT1

	/* 一下屏蔽的代码是GPT定时器中断代码，
	 * 如果想学习GPT定时器的话可以参考一下代码。
	 */
#if 0
	/*
     * GPT的PR寄存器，GPT的分频设置
     * bit11:0  设置分频值，设置为0表示1分频，
     *          以此类推，最大可以设置为0XFFF，也就是最大4096分频
	 */
	GPT1->PR = 65;	//设置为1，即65+1=66分频，因此GPT1时钟为66M/66=1MHz


	 /*
      * GPT的OCR1寄存器，GPT的输出比较1比较计数值，
      * 当GPT的计数值等于OCR1里面值时候，输出比较1就会发生中断
      * 这里定时500ms产生中断，因此就应该为1000000/2=500000;
	  */
	GPT1->OCR[0] = 500000;

	/*
     * GPT的IR寄存器，使能通道1的比较中断
     * bit0： 0 使能输出比较中断
	 */
	GPT1->IR |= 1 << 0;

	/*
     * 使能GIC里面相应的中断，并且注册中断处理函数
	 */
	GIC_EnableIRQ(GPT1_IRQn);	//使能GIC中对应的中断
	system_register_irqhandler(GPT1_IRQn, (system_irq_handler_t)gpt1_irqhandler, NULL);	//注册中断服务函数	
#endif
	
}

 
/*
 * @description		: 微秒(us)级延时
 * @param - value	: 需要延时的us数,最大延时0XFFFFFFFFus
 * @return 			: 无
 */
void udelay(unsigned int usdelay)
{
	unsigned long oldcnt,newcnt;
	unsigned long tcntvalue = 0;	/* 走过的总时间  */

	oldcnt = GPT1->CNT;
	while(1)
	{
		newcnt = GPT1->CNT;
		if(newcnt != oldcnt)
		{
			if(newcnt > oldcnt)		/* GPT是向上计数器,并且没有溢出 */
				tcntvalue += newcnt - oldcnt;
			else  					/* 发生溢出    */
				tcntvalue += 0XFFFFFFFF-oldcnt + newcnt;
			oldcnt = newcnt;
			if(tcntvalue >= usdelay)/* 延时时间到了 */
			break;			 		/*  跳出 */
		}
	}
}

/*
 * @description		: 毫秒(ms)级延时
 * @param - msdelay	: 需要延时的ms数
 * @return 			: 无
 */
void mdelay(unsigned int msdelay)
{
	int i = 0;
	for(i=0; i<msdelay; i++)
	{
		udelay(1000);
	}
}