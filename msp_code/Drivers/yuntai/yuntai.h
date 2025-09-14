#include "ti_msp_dl_config.h"

void biyao_Set_Angle(float duty);
void Set_Bujin_PWM(int Target);
int Calculate_target(int Target);
float Limit_Value_Float(float in, float max, float min);
int32_t Limit_Value(int32_t in, int32_t max, int32_t min);
void yuntai_aim(float dx);
int32_t pid_calculate(float dx);

float angle_to_duty(float angle_error);
void biyao_pid_control(float duty);

#define MID_DUTY 10.0f            // 中位占空比
