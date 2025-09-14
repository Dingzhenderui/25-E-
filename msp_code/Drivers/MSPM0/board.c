#include "board.h"
#include "stdio.h"

#define RE_0_BUFF_LEN_MAX	128

volatile uint8_t  recv0_buff[RE_0_BUFF_LEN_MAX] = {0};
volatile uint16_t recv0_length = 0;
volatile uint8_t  recv0_flag = 0;
volatile unsigned long tick_ms;
volatile uint32_t start_time;

void SysTick_Init(void)
{
    DL_SYSTICK_config(CPUCLK_FREQ/1000);
    NVIC_SetPriority(SysTick_IRQn, 0);
}

void SysTick_Handler(void)
{
    tick_ms++;
}

int mspm0_delay_ms(unsigned long num_ms)
{
    start_time = tick_ms;
    while (tick_ms - start_time < num_ms);
    return 0;
}

int mspm0_get_clock_ms(unsigned long *count)
{
    if (!count)
        return 1;
    count[0] = tick_ms;
    return 0;
}
//搭配滴答定时器实现的精确us延时
void delay_us(unsigned long __us) 
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 38;

    // 计算需要的时钟数 = 延迟微秒数 * 每微秒的时钟数
    ticks = __us * (80000000 / 1000000);

		// 获取当前的SysTick值
		told = SysTick->VAL;

    while (1)
    {
        // 重复刷新获取当前的SysTick值
        tnow = SysTick->VAL;

        if (tnow != told)
        {
            if (tnow < told)
                tcnt += told - tnow;
            else
                tcnt += SysTick->LOAD - tnow + told;

            told = tnow;

            // 如果达到了需要的时钟数，就退出循环
            if (tcnt >= ticks)
                break;
        }
    }
}
//搭配滴答定时器实现的精确ms延时
void delay_ms_new(unsigned long ms) 
{
	delay_us( ms * 1000 );
}

void delay_1us(unsigned long __us){ delay_us(__us); }
void delay_1ms(unsigned long ms){ delay_ms_new(ms); }

// void uart0_send_char(char ch)
// {
// 	while( DL_UART_isBusy(UART_CCP_INST) == true );
// 	DL_UART_Main_transmitData(UART_CCP_INST, ch);

// }
// void uart0_send_string(char* str)
// {
// 	while(*str!=0&&str!=0)
// 	{
// 		uart0_send_char(*str++);
// 	}
// }


#if !defined(__MICROLIB)
#if (__ARMCLIB_VERSION <= 6000000)
struct __FILE
{
	int handle;
};
#endif

FILE __stdout;

void _sys_exit(int x)
{
	x = x;
}
#endif


// int fputc(int ch, FILE *stream)
// {
// 	while( DL_UART_isBusy(UART_CCP_INST) == true );
	
// 	DL_UART_Main_transmitData(UART_CCP_INST, ch);
	
// 	return ch;
// }

