#include "main.h"
#include "pid.h"
#include "yuntai.h"

extern int16_t dx, dy;           // 坐标值
extern uint8_t ready;            // 数据就绪标志（关键：只有ready=1时，dx/d才有效）
uint8_t aim_flag = 0;
extern uint8_t d; 
uint8_t f=0;
// 新增：连续有效瞄准计数（过滤K230的偶尔误判）
static uint8_t stable_aim_count = 0; 
#define STABLE_THRESHOLD 3        // 连续3次满足条件即判定为瞄准成功
uint32_t timer_count = 0;
uint8_t find = 0;
uint8_t button_S1 = 0;  // S1按键计数
uint8_t button_S2 = 0;  // S2按键计数
static int16_t dx_history[3] = {0, 0, 0};  // 历史dx缓存
static uint8_t history_idx = 0;            // 缓存索引
static uint8_t valid_frame_cnt = 0;       // 连续有效帧计数器
static float rotate_total_angle = 0.0f;   // 累计旋转角度
uint8_t lost_frame_cnt = 0;
uint16_t biyaoconunt=0;
// 新增非阻塞延时变量
static uint32_t rotate_timer = 0;
static uint8_t rotate_dir = 0;  // 0=正转,1=反转
int main(void)
{
    SYSCFG_DL_init();
    SysTick_Init();
    
    NVIC_ClearPendingIRQ(TIMER_0_INST_INT_IRQN);  // 清除 pending
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);        // 使能TIMER0中断
    DL_TimerA_startCounter(PWM_dizuo_INST);
    
    DL_TimerG_startCounter(PWM_biyao_INST);

    WIT_Init();
    DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 初始关闭蓝光
    NVIC_ClearPendingIRQ(UART_K230_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_K230_INST_INT_IRQN);   // 使能K230的UART中断


    while(DL_GPIO_readPins(GPIO_BUTTON_PIN_S2_PORT, GPIO_BUTTON_PIN_S2_PIN))
    {key_scan_count1();}
    
    DL_TimerG_startCounter(TIMER_0_INST);
    // DL_UART_transmitDataBlocking(UART_K230_INST, 'b'); 

    // biyao_Set_Angle(10.19);
    

    
    while (1) 
    {
        
        if(button_S1 == 1)
        { 
                biyao_Set_Angle(10.17);
                if(timer_count >= 180)
                {
                    DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                    delay_1ms(500);
                    DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                    Set_Bujin_PWM(0);
                    while(1);
                }
                // biyao_pid_control(dy);
                // 仅当有新数据时（ready=1）才更新瞄准状态，避免使用旧数据
                if (ready) 
                {
                    ready = 0; // 立即清零，防止重复处理

                    // 情况1：K230识别到目标（d=0，有效帧）
                    if (d == 0) 
                    {
                        // 检查dx是否在瞄准范围内（-2~2）
                        if (dx >= -2 && dx <= 2) 
                        {
                            stable_aim_count++; // 连续满足，计数+1
                            // 连续STABLE_THRESHOLD次满足，判定为瞄准成功
                            if (stable_aim_count >= STABLE_THRESHOLD) 
                            {
                                aim_flag = 1;
                            }
                        } 
                        else 
                        {
                            // dx超出范围，重置计数
                            stable_aim_count = 0;
                            aim_flag = 0;
                        }
                    } 
                    // 情况2：K230未识别到目标（d=1，#帧），直接重置
                    else if (d == 1) 
                    {
                        stable_aim_count = 0;
                        aim_flag = 0;
                    }
                }

                // 根据瞄准状态执行动作
                if (!aim_flag)
                {
                    // 未瞄准：控制云台调整
                    yuntai_aim(dx);
                    // biyao_pid_control(dy);
                    DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                }
                else
                {
                    if(!f)
                    {// 瞄准成功：停止云台，打开蓝光（常亮，方便观察）
                    Set_Bujin_PWM(0);
                    DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                    delay_1ms(500);
                    DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                    f=1;
                    while(1);
                    // 若需要闪烁，可保留delay，但可能导致主循环阻塞，建议用定时器
                    // delay_1ms(500);
                    // DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN);
                    // delay_1ms(500);
                    }
                }
            
            
        }
        else if (button_S1 == 2) 
        {
            while(1)
            {
                    while(find==0)
                    {
                        static uint32_t last_rotate_time = 0;  

                        // 每2000ms切换方向 → 2000 ÷ 10 = 200（timer_count间隔200）
                        if (timer_count - last_rotate_time > 200)  
                        {
                            rotate_dir = !rotate_dir;               
                            last_rotate_time = timer_count;         
                        }

                        if(rotate_dir == 0)
                            Set_Bujin_PWM(Calculate_target(10));  
                        else
                            Set_Bujin_PWM(Calculate_target(-10));


                        if (ready) {  // 有新的识别数据
                            ready = 0;
                            int16_t current_dxx = dx;  // 记录当前帧dx

                            // 1. 更新dx历史缓存（循环存储最近3帧）
                            dx_history[history_idx] = current_dxx;
                            history_idx = (history_idx + 1) % 3;  // 索引循环

                            // 2. 判断最近3帧dx是否均不相等（排除重复的旧数据）
                            bool is_new_data = true;
                            // 至少有3帧数据后才判断（前2帧不做判断）
                            if (history_idx == 0) {  // 缓存满3帧后才检查
                                // 检查3帧是否全不相等
                                if (dx_history[0] == dx_history[1] || 
                                    dx_history[1] == dx_history[2] || 
                                    dx_history[0] == dx_history[2]) {
                                    is_new_data = false;  // 有重复，视为旧数据
                                }
                            }

                            // 3. 仅新数据参与有效帧计数
                            if (d == 0) {  // 本帧识别到矩形
                                // 只有新数据才累加计数
                                if (is_new_data || history_idx < 2) {  // 前2帧直接计数（缓存未满）
                                    valid_frame_cnt++;
                                    // 连续2帧有效，确认目标
                                    if (valid_frame_cnt >= 2) {
                                        find = 1;
                                        Set_Bujin_PWM(0);
                                        rotate_total_angle = 0;
                                        valid_frame_cnt = 0;
                                        printf("识别到矩形find=1\n");
                                    }
                                } else {
                                    // 旧数据（重复），重置计数
                                    valid_frame_cnt = 0;
                                    printf("检测到重复旧数据，重置计数\n");
                                }
                            } else {  // 本帧未识别到
                                valid_frame_cnt = 0;
                            }
                        }
                    }
                    while(find==1)
                    {
                        if (ready) 
                        {
                                ready = 0; // 立即清零，防止重复处理
                                
                                // 情况1：K230识别到目标（d=0，有效帧）
                                if (d == 0) 
                                {
                                    lost_frame_cnt = 0;  // 识别到目标，重置丢失计数
                                    // 检查dx是否在瞄准范围内（-2~2）
                                    if (dx >= -2 && dx <= 2) 
                                    {
                                        stable_aim_count++; // 连续满足，计数+1
                                        // 连续STABLE_THRESHOLD次满足，判定为瞄准成功
                                        if (stable_aim_count >= STABLE_THRESHOLD) 
                                        {
                                            aim_flag = 1;
                                        }
                                    } 
                                    else 
                                    {
                                        // dx超出范围，重置计数
                                        stable_aim_count = 0;
                                        aim_flag = 0;
                                    }
                                } 
                                // 情况2：K230未识别到目标（d=1，#帧），直接重置
                                else if (d == 1) 
                                {
                                    lost_frame_cnt++;
                                    // 连续3帧丢失，重置为搜索状态
                                    if(lost_frame_cnt >= 3)
                                    {
                                        find = 0;          // 重新搜索
                                        aim_flag = 0;      // 重置瞄准状态
                                        f = 0;             // 重置瞄准成功标志
                                        lost_frame_cnt = 0;
                                        break;  // 退出find==1循环
                                    }
                                }
                            }
                            // 根据瞄准状态执行动作
                            if (!aim_flag)
                            {
                                // 未瞄准：控制云台调整
                                yuntai_aim(dx);
                                // biyao_pid_control(dy);
                                DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                            }
                            else
                            {
                                if(!f)
                                {// 瞄准成功：停止云台，打开蓝光（常亮，方便观察）
                                Set_Bujin_PWM(0);
                                DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                                f=1;
                                while(1)
                                {
                                    yuntai_aim(dx);
                                }
                                }
                            }

                    }
                }
                
                

        }
        
        else if (button_S1 == 3) //shun hou
        {
            biyao_Set_Angle(10.19);
            while(1)
            {
                if(timer_count >= 380) 
                {
                    Set_Bujin_PWM(0);
                    DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                    delay_1ms(500);
                    DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                    
                    // DL_GPIO_clearPins(GPIO_LED_PORT,GPIO_LED_PIN_LED_PIN);
                    while(1);
                }
                else
                {
                    while(find==0)
                    {
                        if(timer_count >= 380) 
                        {
                            Set_Bujin_PWM(0);
                            DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                            delay_1ms(500);
                            DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                            
                            // DL_GPIO_clearPins(GPIO_LED_PORT,GPIO_LED_PIN_LED_PIN);
                            while(1);
                        }
                        static uint32_t last_rotate_time = 0;  

                        // 每2000ms切换方向 → 2000 ÷ 10 = 200（timer_count间隔200）
                        if (timer_count - last_rotate_time > 250)  
                        {
                            rotate_dir = !rotate_dir;               
                            last_rotate_time = timer_count;         
                        }

                        if(rotate_dir == 0)
                            Set_Bujin_PWM(Calculate_target(10));  
                        else
                            Set_Bujin_PWM(Calculate_target(10));


                        if (ready) {  // 有新的识别数据
                            ready = 0;
                            int16_t current_dxx = dx;  // 记录当前帧dx

                            // 1. 更新dx历史缓存（循环存储最近3帧）
                            dx_history[history_idx] = current_dxx;
                            history_idx = (history_idx + 1) % 3;  // 索引循环

                            // 2. 判断最近3帧dx是否均不相等（排除重复的旧数据）
                            bool is_new_data = true;
                            // 至少有3帧数据后才判断（前2帧不做判断）
                            if (history_idx == 0) {  // 缓存满3帧后才检查
                                // 检查3帧是否全不相等
                                if (dx_history[0] == dx_history[1] || 
                                    dx_history[1] == dx_history[2] || 
                                    dx_history[0] == dx_history[2]) {
                                    is_new_data = false;  // 有重复，视为旧数据
                                }
                            }

                            // 3. 仅新数据参与有效帧计数
                            if (d == 0) {  // 本帧识别到矩形
                                // 只有新数据才累加计数
                                if (is_new_data || history_idx < 2) {  // 前2帧直接计数（缓存未满）
                                    valid_frame_cnt++;
                                    // 连续2帧有效，确认目标
                                    if (valid_frame_cnt >= 2) {
                                        find = 1;
                                        Set_Bujin_PWM(0);
                                        rotate_total_angle = 0;
                                        valid_frame_cnt = 0;
                                        
                                    }
                                } else {
                                    // 旧数据（重复），重置计数
                                    valid_frame_cnt = 0;
                                    
                                }
                            } else {  // 本帧未识别到
                                valid_frame_cnt = 0;
                            }
                        }
                    }
                    while(find==1)
                    {
                        if(timer_count >= 380) 
                        {
                            Set_Bujin_PWM(0);
                            DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                            delay_1ms(500);
                            DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                            
                            // DL_GPIO_clearPins(GPIO_LED_PORT,GPIO_LED_PIN_LED_PIN);
                            while(1);
                        }
                        if (ready) 
                        {
                                ready = 0; // 立即清零，防止重复处理
                                
                                // 情况1：K230识别到目标（d=0，有效帧）
                                if (d == 0) 
                                {
                                    lost_frame_cnt = 0;  // 识别到目标，重置丢失计数
                                    // 检查dx是否在瞄准范围内（-2~2）
                                    if (dx >= -2 && dx <= 2&& dy<3 && dy>-3) 
                                    {
                                        stable_aim_count++; // 连续满足，计数+1
                                        // 连续STABLE_THRESHOLD次满足，判定为瞄准成功
                                        if (stable_aim_count >= STABLE_THRESHOLD) 
                                        {
                                            aim_flag = 1;
                                        }
                                    } 
                                    else 
                                    {
                                        // dx超出范围，重置计数
                                        stable_aim_count = 0;
                                        aim_flag = 0;
                                    }
                                } 
                                // 情况2：K230未识别到目标（d=1，#帧），直接重置
                                else if (d == 1) 
                                {
                                    lost_frame_cnt++;
                                    // 连续3帧丢失，重置为搜索状态
                                    if(lost_frame_cnt >= 3)
                                    {
                                        find = 0;          // 重新搜索
                                        aim_flag = 0;      // 重置瞄准状态
                                        f = 0;             // 重置瞄准成功标志
                                        lost_frame_cnt = 0;
                                        break;  // 退出find==1循环
                                    }
                                }
                            }
                            // 根据瞄准状态执行动作
                            if (!aim_flag)
                            {
                                // 未瞄准：控制云台调整
                                yuntai_aim(dx);
                            
                                // biyao_pid_control(dy);
                                DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                            }
                            else
                            {
                                if(!f)
                                {// 瞄准成功：停止云台，打开蓝光（常亮，方便观察）
                                Set_Bujin_PWM(0);
                                DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                                delay_1ms(500);
                                DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                                f=1;
                                while(1);
                                }
                            }

                    }
                }
                
                

            }
        }
        else if (button_S1 == 4)//ni hou        
        {
            biyao_Set_Angle(10.19);
            while(1)
            {
                if(timer_count >= 380) 
                {
                    Set_Bujin_PWM(0);
                    DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                    delay_1ms(500);
                    DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                    
                    // DL_GPIO_clearPins(GPIO_LED_PORT,GPIO_LED_PIN_LED_PIN);
                    while(1);
                }
                else
                {
                    while(find==0)
                    {
                        if(timer_count >= 380) 
                        {
                            Set_Bujin_PWM(0);
                            DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                            delay_1ms(500);
                            DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                            
                            // DL_GPIO_clearPins(GPIO_LED_PORT,GPIO_LED_PIN_LED_PIN);
                            while(1);
                        }
                        static uint32_t last_rotate_time = 0;  

                        // 每2000ms切换方向 → 2000 ÷ 10 = 200（timer_count间隔200）
                        if (timer_count - last_rotate_time > 250)  
                        {
                            rotate_dir = !rotate_dir;               
                            last_rotate_time = timer_count;         
                        }

                        if(rotate_dir == 0)
                            Set_Bujin_PWM(Calculate_target(-10));  
                        else
                            Set_Bujin_PWM(Calculate_target(10));


                        if (ready) {  // 有新的识别数据
                            ready = 0;
                            int16_t current_dxx = dx;  // 记录当前帧dx

                            // 1. 更新dx历史缓存（循环存储最近3帧）
                            dx_history[history_idx] = current_dxx;
                            history_idx = (history_idx + 1) % 3;  // 索引循环

                            // 2. 判断最近3帧dx是否均不相等（排除重复的旧数据）
                            bool is_new_data = true;
                            // 至少有3帧数据后才判断（前2帧不做判断）
                            if (history_idx == 0) {  // 缓存满3帧后才检查
                                // 检查3帧是否全不相等
                                if (dx_history[0] == dx_history[1] || 
                                    dx_history[1] == dx_history[2] || 
                                    dx_history[0] == dx_history[2]) {
                                    is_new_data = false;  // 有重复，视为旧数据
                                }
                            }

                            // 3. 仅新数据参与有效帧计数
                            if (d == 0) {  // 本帧识别到矩形
                                // 只有新数据才累加计数
                                if (is_new_data || history_idx < 2) {  // 前2帧直接计数（缓存未满）
                                    valid_frame_cnt++;
                                    // 连续2帧有效，确认目标
                                    if (valid_frame_cnt >= 2) {
                                        find = 1;
                                        Set_Bujin_PWM(0);
                                        rotate_total_angle = 0;
                                        valid_frame_cnt = 0;
                                        printf("识别到矩形find=1\n");
                                    }
                                } else {
                                    // 旧数据（重复），重置计数
                                    valid_frame_cnt = 0;
                                    printf("检测到重复旧数据，重置计数\n");
                                }
                            } else {  // 本帧未识别到
                                valid_frame_cnt = 0;
                            }
                        }
                    }
                    while(find==1)
                    {
                        if(timer_count >= 380) 
                        {
                            Set_Bujin_PWM(0);
                            DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                            delay_1ms(500);
                            DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                            
                            // DL_GPIO_clearPins(GPIO_LED_PORT,GPIO_LED_PIN_LED_PIN);
                            while(1);
                        }
                        if (ready) 
                        {
                                ready = 0; // 立即清零，防止重复处理
                                
                                // 情况1：K230识别到目标（d=0，有效帧）
                                if (d == 0) 
                                {
                                    lost_frame_cnt = 0;  // 识别到目标，重置丢失计数
                                    // 检查dx是否在瞄准范围内（-2~2）
                                    if (dx >= -2 && dx <= 2) 
                                    {
                                        stable_aim_count++; // 连续满足，计数+1
                                        // 连续STABLE_THRESHOLD次满足，判定为瞄准成功
                                        if (stable_aim_count >= STABLE_THRESHOLD) 
                                        {
                                            aim_flag = 1;
                                        }
                                    } 
                                    else 
                                    {
                                        // dx超出范围，重置计数
                                        stable_aim_count = 0;
                                        aim_flag = 0;
                                    }
                                } 
                                // 情况2：K230未识别到目标（d=1，#帧），直接重置
                                else if (d == 1) 
                                {
                                    lost_frame_cnt++;
                                    // 连续3帧丢失，重置为搜索状态
                                    if(lost_frame_cnt >= 3)
                                    {
                                        find = 0;          // 重新搜索
                                        aim_flag = 0;      // 重置瞄准状态
                                        f = 0;             // 重置瞄准成功标志
                                        lost_frame_cnt = 0;
                                        break;  // 退出find==1循环
                                    }
                                }
                            }
                            // 根据瞄准状态执行动作
                            if (!aim_flag)
                            {
                                // 未瞄准：控制云台调整
                                yuntai_aim(dx);
                                // biyao_pid_control(dy);
                                DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                            }
                            else
                            {
                                if(!f)
                                {// 瞄准成功：停止云台，打开蓝光（常亮，方便观察）
                                Set_Bujin_PWM(0);
                                DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                                delay_1ms(500);
                                DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                                f=1;
                                while(1);
                                }
                            }

                    }
                }
                
                

            }
        }
        else if (button_S1 == 5)//shun qian
        {
            biyao_Set_Angle(10.15);
            while(1)
            {
                if(timer_count >= 380) 
                {
                    Set_Bujin_PWM(0);
                    DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                    delay_1ms(500);
                    DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                    
                    // DL_GPIO_clearPins(GPIO_LED_PORT,GPIO_LED_PIN_LED_PIN);
                    while(1);
                }
                else
                {
                    while(find==0)
                    {
                        if(timer_count >= 380) 
                        {
                            Set_Bujin_PWM(0);
                            DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                            delay_1ms(500);
                            DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                            
                            // DL_GPIO_clearPins(GPIO_LED_PORT,GPIO_LED_PIN_LED_PIN);
                            while(1);
                        }
                        static uint32_t last_rotate_time = 0;  

                        // 每2000ms切换方向 → 2000 ÷ 10 = 200（timer_count间隔200）
                        if (timer_count - last_rotate_time > 250)  
                        {
                            rotate_dir = !rotate_dir;               
                            last_rotate_time = timer_count;         
                        }

                        if(rotate_dir == 0)
                            Set_Bujin_PWM(Calculate_target(10));  
                        else
                            Set_Bujin_PWM(Calculate_target(10));


                        if (ready) {  // 有新的识别数据
                            ready = 0;
                            int16_t current_dxx = dx;  // 记录当前帧dx

                            // 1. 更新dx历史缓存（循环存储最近3帧）
                            dx_history[history_idx] = current_dxx;
                            history_idx = (history_idx + 1) % 3;  // 索引循环

                            // 2. 判断最近3帧dx是否均不相等（排除重复的旧数据）
                            bool is_new_data = true;
                            // 至少有3帧数据后才判断（前2帧不做判断）
                            if (history_idx == 0) {  // 缓存满3帧后才检查
                                // 检查3帧是否全不相等
                                if (dx_history[0] == dx_history[1] || 
                                    dx_history[1] == dx_history[2] || 
                                    dx_history[0] == dx_history[2]) {
                                    is_new_data = false;  // 有重复，视为旧数据
                                }
                            }

                            // 3. 仅新数据参与有效帧计数
                            if (d == 0) {  // 本帧识别到矩形
                                // 只有新数据才累加计数
                                if (is_new_data || history_idx < 2) {  // 前2帧直接计数（缓存未满）
                                    valid_frame_cnt++;
                                    // 连续2帧有效，确认目标
                                    if (valid_frame_cnt >= 2) {
                                        find = 1;
                                        Set_Bujin_PWM(0);
                                        rotate_total_angle = 0;
                                        valid_frame_cnt = 0;
                                        
                                    }
                                } else {
                                    // 旧数据（重复），重置计数
                                    valid_frame_cnt = 0;
                                    
                                }
                            } else {  // 本帧未识别到
                                valid_frame_cnt = 0;
                            }
                        }
                    }
                    while(find==1)
                    {
                        if(timer_count >= 380) 
                        {
                            Set_Bujin_PWM(0);
                            DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                            delay_1ms(500);
                            DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                            
                            // DL_GPIO_clearPins(GPIO_LED_PORT,GPIO_LED_PIN_LED_PIN);
                            while(1);
                        }
                        if (ready) 
                        {
                                ready = 0; // 立即清零，防止重复处理
                                
                                // 情况1：K230识别到目标（d=0，有效帧）
                                if (d == 0) 
                                {
                                    lost_frame_cnt = 0;  // 识别到目标，重置丢失计数
                                    // 检查dx是否在瞄准范围内（-2~2）
                                    if (dx >= -2 && dx <= 2&& dy<3 && dy>-3) 
                                    {
                                        stable_aim_count++; // 连续满足，计数+1
                                        // 连续STABLE_THRESHOLD次满足，判定为瞄准成功
                                        if (stable_aim_count >= STABLE_THRESHOLD) 
                                        {
                                            aim_flag = 1;
                                        }
                                    } 
                                    else 
                                    {
                                        // dx超出范围，重置计数
                                        stable_aim_count = 0;
                                        aim_flag = 0;
                                    }
                                } 
                                // 情况2：K230未识别到目标（d=1，#帧），直接重置
                                else if (d == 1) 
                                {
                                    lost_frame_cnt++;
                                    // 连续3帧丢失，重置为搜索状态
                                    if(lost_frame_cnt >= 3)
                                    {
                                        find = 0;          // 重新搜索
                                        aim_flag = 0;      // 重置瞄准状态
                                        f = 0;             // 重置瞄准成功标志
                                        lost_frame_cnt = 0;
                                        break;  // 退出find==1循环
                                    }
                                }
                            }
                            // 根据瞄准状态执行动作
                            if (!aim_flag)
                            {
                                // 未瞄准：控制云台调整
                                yuntai_aim(dx);
                            
                                // biyao_pid_control(dy);
                                DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                            }
                            else
                            {
                                if(!f)
                                {// 瞄准成功：停止云台，打开蓝光（常亮，方便观察）
                                Set_Bujin_PWM(0);
                                DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                                delay_1ms(500);
                                DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                                f=1;
                                while(1);
                                }
                            }

                    }
                }
                
                

            }
        }
        else if (button_S1 == 6)//ni qian
        {
            biyao_Set_Angle(10.15);
            while(1)
            {
                if(timer_count >= 380) 
                {
                    Set_Bujin_PWM(0);
                    DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                    delay_1ms(500);
                    DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                    
                    // DL_GPIO_clearPins(GPIO_LED_PORT,GPIO_LED_PIN_LED_PIN);
                    while(1);
                }
                else
                {
                    while(find==0)
                    {
                        if(timer_count >= 380) 
                        {
                            Set_Bujin_PWM(0);
                            DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                            delay_1ms(500);
                            DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                            
                            // DL_GPIO_clearPins(GPIO_LED_PORT,GPIO_LED_PIN_LED_PIN);
                            while(1);
                        }
                        static uint32_t last_rotate_time = 0;  

                        // 每2000ms切换方向 → 2000 ÷ 10 = 200（timer_count间隔200）
                        if (timer_count - last_rotate_time > 250)  
                        {
                            rotate_dir = !rotate_dir;               
                            last_rotate_time = timer_count;         
                        }

                        if(rotate_dir == 0)
                            Set_Bujin_PWM(Calculate_target(-10));  
                        else
                            Set_Bujin_PWM(Calculate_target(10));


                        if (ready) {  // 有新的识别数据
                            ready = 0;
                            int16_t current_dxx = dx;  // 记录当前帧dx

                            // 1. 更新dx历史缓存（循环存储最近3帧）
                            dx_history[history_idx] = current_dxx;
                            history_idx = (history_idx + 1) % 3;  // 索引循环

                            // 2. 判断最近3帧dx是否均不相等（排除重复的旧数据）
                            bool is_new_data = true;
                            // 至少有3帧数据后才判断（前2帧不做判断）
                            if (history_idx == 0) {  // 缓存满3帧后才检查
                                // 检查3帧是否全不相等
                                if (dx_history[0] == dx_history[1] || 
                                    dx_history[1] == dx_history[2] || 
                                    dx_history[0] == dx_history[2]) {
                                    is_new_data = false;  // 有重复，视为旧数据
                                }
                            }

                            // 3. 仅新数据参与有效帧计数
                            if (d == 0) {  // 本帧识别到矩形
                                // 只有新数据才累加计数
                                if (is_new_data || history_idx < 2) {  // 前2帧直接计数（缓存未满）
                                    valid_frame_cnt++;
                                    // 连续2帧有效，确认目标
                                    if (valid_frame_cnt >= 2) {
                                        find = 1;
                                        Set_Bujin_PWM(0);
                                        rotate_total_angle = 0;
                                        valid_frame_cnt = 0;
                                        printf("识别到矩形find=1\n");
                                    }
                                } else {
                                    // 旧数据（重复），重置计数
                                    valid_frame_cnt = 0;
                                    printf("检测到重复旧数据，重置计数\n");
                                }
                            } else {  // 本帧未识别到
                                valid_frame_cnt = 0;
                            }
                        }
                    }
                    while(find==1)
                    {
                        if(timer_count >= 380) 
                        {
                            Set_Bujin_PWM(0);
                            DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                            delay_1ms(500);
                            DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                            
                            // DL_GPIO_clearPins(GPIO_LED_PORT,GPIO_LED_PIN_LED_PIN);
                            while(1);
                        }
                        if (ready) 
                        {
                                ready = 0; // 立即清零，防止重复处理
                                
                                // 情况1：K230识别到目标（d=0，有效帧）
                                if (d == 0) 
                                {
                                    lost_frame_cnt = 0;  // 识别到目标，重置丢失计数
                                    // 检查dx是否在瞄准范围内（-2~2）
                                    if (dx >= -2 && dx <= 2) 
                                    {
                                        stable_aim_count++; // 连续满足，计数+1
                                        // 连续STABLE_THRESHOLD次满足，判定为瞄准成功
                                        if (stable_aim_count >= STABLE_THRESHOLD) 
                                        {
                                            aim_flag = 1;
                                        }
                                    } 
                                    else 
                                    {
                                        // dx超出范围，重置计数
                                        stable_aim_count = 0;
                                        aim_flag = 0;
                                    }
                                } 
                                // 情况2：K230未识别到目标（d=1，#帧），直接重置
                                else if (d == 1) 
                                {
                                    lost_frame_cnt++;
                                    // 连续3帧丢失，重置为搜索状态
                                    if(lost_frame_cnt >= 3)
                                    {
                                        find = 0;          // 重新搜索
                                        aim_flag = 0;      // 重置瞄准状态
                                        f = 0;             // 重置瞄准成功标志
                                        lost_frame_cnt = 0;
                                        break;  // 退出find==1循环
                                    }
                                }
                            }
                            // 根据瞄准状态执行动作
                            if (!aim_flag)
                            {
                                // 未瞄准：控制云台调整
                                yuntai_aim(dx);
                                // biyao_pid_control(dy);
                                DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                            }
                            else
                            {
                                if(!f)
                                {// 瞄准成功：停止云台，打开蓝光（常亮，方便观察）
                                Set_Bujin_PWM(0);
                                DL_GPIO_setPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 常亮蓝光
                                delay_1ms(500);
                                DL_GPIO_clearPins(GPIO_BlueLight_PORT, GPIO_BlueLight_PIN_BlueLight_PIN); // 关闭蓝光
                                f=1;
                                while(1);
                                }
                            }

                    }
                }
                
                

            }
        }

        
    }
}


void TIMER_0_INST_IRQHandler(void)
{
    // DL_TimerG_clearInterruptFlag(TIMER_0_INST, DL_TIMER_IIDX_ZERO);
    // switch (DL_TimerG_getPendingInterrupt(TIMER_0_INST)) {
    //     case DL_TIMER_IIDX_ZERO:
            timer_count++;
            // printf("%d\n",timer_count);
    //         break;
    //     default:
    //         break;
    // }
}


void key_scan_count2(void)
{
  if(!DL_GPIO_readPins(GPIO_BUTTON_PIN_S2_PORT, GPIO_BUTTON_PIN_S2_PIN))
  {

    delay_1ms(100);
    if(!DL_GPIO_readPins(GPIO_BUTTON_PIN_S2_PORT, GPIO_BUTTON_PIN_S2_PIN))
    {
      button_S2 += 1;
      DL_GPIO_clearPins(GPIO_LED_PORT, GPIO_LED_PIN_LED_PIN);
      delay_1ms(100);
      DL_GPIO_setPins(GPIO_LED_PORT, GPIO_LED_PIN_LED_PIN);
    }
    
  }
}
void key_scan_count1(void)
{
  if(DL_GPIO_readPins(GPIO_BUTTON_PIN_S1_PORT, GPIO_BUTTON_PIN_S1_PIN))
  {

    delay_1ms(100);
    if(DL_GPIO_readPins(GPIO_BUTTON_PIN_S1_PORT, GPIO_BUTTON_PIN_S1_PIN))
    {
      button_S1 += 1;
      DL_GPIO_clearPins(GPIO_LED_PORT, GPIO_LED_PIN_LED_PIN);
      delay_1ms(100);
      DL_GPIO_setPins(GPIO_LED_PORT, GPIO_LED_PIN_LED_PIN);
    }
    
  }
}


