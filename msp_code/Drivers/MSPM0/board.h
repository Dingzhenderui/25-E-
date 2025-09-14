#include "ti_msp_dl_config.h"
#include "string.h"
#include "math.h"

#define MOTOR_STOP    0
#define MOTOR_SPEED   1
#define MOTOR_DIR  		2
#define MOTOR_TRACK  	3

#define ABS(a)      (a>0 ? a:(-a))

void delay_us(unsigned long __us);
void delay_ms_new(unsigned long ms);
void delay_1us(unsigned long __us);
void delay_1ms(unsigned long ms);

void uart0_send_char(char ch);
void uart0_send_string(char* str);

void SysTick_Init(void);

extern volatile unsigned long tick_ms;

int mspm0_delay_ms(unsigned long num_ms);
int mspm0_get_clock_ms(unsigned long *count);

