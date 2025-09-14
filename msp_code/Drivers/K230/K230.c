#include "K230.h"
#include <string.h>
#include <stdlib.h>

// 缓冲区（足够存"AA,-32768,32767,FF"）
char rx_buf[20] = {0};
uint8_t ptr = 0;          // 缓冲区指针
uint8_t state = 0;        // 0:等帧头AA, 1:收数据, 2:等帧尾FF
int16_t dx, dy;           // 坐标值
uint8_t ready = 0;        // 数据就绪标志
uint8_t d = 0;  
// UART中断处理：接收"AA,dx,dy,FF"
void UART_K230_INST_IRQHandler(void) {
    if (DL_UART_Main_getPendingInterrupt(UART_K230_INST) == DL_UART_MAIN_IIDX_RX) {
        char c = DL_UART_Main_receiveData(UART_K230_INST);

        if (c == '#'&& state == 0) {
            dx = 0;       // dx置为0
            dy = 0;       // dy置为0
            d = 1;
            ready = 1;    // 置位就绪标志，通知主程序
            // 重置接收状态，避免干扰后续帧解析
            ptr = 0;
            state = 0;
            memset(rx_buf, 0, 20);
            return; // 直接返回，不执行后续状态机逻辑
        }
         
        switch(state) {
            case 0: // 等帧头"AA"
                d = 0;
                if (ptr==0 && c=='A') ptr=1;
                else if (ptr==1 && c=='A') { state=1; ptr=2; rx_buf[0]='A'; rx_buf[1]='A'; }
                else ptr=0; // 不匹配则重置
                break;

            case 1: // 收数据（直到遇到帧尾前的逗号）
                if (ptr<19) rx_buf[ptr++]=c;
                if (c==',' && strchr(rx_buf+2, ',')!=NULL) state=2; // 检测到第二个逗号
                break;

            case 2: // 等帧尾"FF"
                if (ptr<19) rx_buf[ptr++]=c;
                if (ptr>=4 && rx_buf[ptr-2]=='F' && rx_buf[ptr-1]=='F') { // 帧尾完整
                    // 解析dx和dy
                    char* p1 = strchr(rx_buf, ',')+1;
                    char* p2 = strchr(p1, ',')+1;
                    dx = atoi(p1);
                    dy = atoi(p2);
                    ready = 1;
                    // 重置
                    ptr=0; state=0; memset(rx_buf,0,20);
                }
                break;
        }
    }
}