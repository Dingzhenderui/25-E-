#include "yuntai.h"
#include "board.h"
#include <math.h>
#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"

extern int16_t dx, dy;      // 坐标偏差值（像素）

// --------------------------
// 步进电机控制参数与函数
// --------------------------
// 基础参数
#define STEPS_PER_REVOLUTION 200  // 电机固有步数/圈（1.8°/步）
#define MICROSTEPS 32             // 细分数（需与驱动器一致）
#define PWM_CLOCK_FREQ 800000     // PWM时钟频率（Hz）
#define ANGLE_PER_STEP (360.0f / (STEPS_PER_REVOLUTION * MICROSTEPS))  // 每步角度（0.05625°）

// PID参数（含积分项）
// #define POSITION_KP 2.0f          // 比例系数
#define POSITION_KD 12.0f          // 微分系数
#define POSITION_KI 0.0002f          // 积分系数（新增）
#define MAX_INTEGRAL 0.05f         // 积分项最大限幅（防止饱和，新增）
#define INTEGRAL_ENABLE_THRESHOLD 6.0f  // 积分启用阈值（偏差<5°时生效，新增）
#define MAX_STEPS 500             // 最大 单步调整步数
#define MIN_STEPS -500            // 最小单步调整步数
#define PIXEL_TO_ANGLE 0.006f     // 像素到角度转换系数
#define DEAD_ZONE_ANGLE 0.015f     // 角度死区

// 分段控制参数
#define SMALL_DX_THRESHOLD 15     // 小dx阈值（≤20像素）
#define KP_SMALL 1.0f             // 小dx比例系数
#define KP_LARGE 2.0f             // 大dx比例系数
#define MAX_SPEED_SMALL 150       // 小dx最大转速（rpm）

// 加减速参数
#define MAX_SPEED 300             // 最大转速（rpm）
#define MIN_SPEED 0               // 最小转速（rpm）
#define MAX_ACCELERATION 50       // 最大加速度（rpm/控制周期）

// 步进电机全局变量
static float last_dx = 0;         // 上一次dx偏差（像素）
static int32_t current_steps = 0; // 当前累计步数
static int current_speed = 0;     // 当前转速（rpm）
static float integral_sum = 0.0f; // 积分项累积值（新增）



// --------------------------
// 舵机控制参数与函数
// --------------------------
// 基础参数
#define MID_DUTY 10.05f            // 中位占空比
#define MIN_DUTY 9.94f             // 最小占空比
#define MAX_DUTY 10.2f            // 最大占空比
#define DUTY_PER_DEG (5.0f/180.0f) // 每度占空比变化

// PID参数（含积分项）
#define BIYAO_KP 0.1f             // 比例系数
#define BIYAO_KD 15.0f             // 微分系数
#define BIYAO_KI 0.002f            // 积分系数（新增）
// #define MAX_ANGLE_INTEGRAL 4.0f   // 舵机积分限幅（新增）
#define DEAD_ZONE 0.005f            // 舵机死区（度）

// 舵机全局变量
static float last_angle_error = 0.0f; // 上一次角度偏差
static float current_duty = MID_DUTY; // 当前占空比
static float integral_angle = 0.0f;   // 舵机积分项累积值（新增）

// --------------------------
// 通用限幅函数
// --------------------------
int32_t Limit_Value(int32_t in, int32_t max, int32_t min) 
{
  if (in > max) return max;
  if (in < min) return min;
  return in;
}

float Limit_Value_Float(float in, float max, float min) {
  if (in > max) return max;
  if (in < min) return min;
  return in;
}

// --------------------------
// 步进电机PID计算（含积分项）
// --------------------------
int32_t pid_calculate(float dx) {
  float angle_error = dx * PIXEL_TO_ANGLE;
  
  // 死区处理：清空积分，停止动作
  if (fabs(angle_error) < DEAD_ZONE_ANGLE) {
    last_dx = dx;
    integral_sum = 0;  // 死区内重置积分
    return 0;
  }
  
  // 选择比例系数（分段控制）
  float kp = (fabs(dx) <= SMALL_DX_THRESHOLD) ? KP_SMALL : KP_LARGE;
  
  // 比例项
  float kp_term = kp * angle_error;
  
  // 微分项
  float kd_term = POSITION_KD * (angle_error - last_dx * PIXEL_TO_ANGLE);
  
  // 积分项（积分分离：仅小偏差时累积）
  float ki_term = 0.0f;
  if (fabs(angle_error) < INTEGRAL_ENABLE_THRESHOLD) {
    integral_sum += angle_error * POSITION_KI;  // 累积积分
    integral_sum = Limit_Value_Float(integral_sum, MAX_INTEGRAL, -MAX_INTEGRAL); // 限幅
    ki_term = integral_sum;
  } else {
    integral_sum = 0;  // 大偏差时清空积分
  }
  
  // 总角度输出
  float angle_output = kp_term + kd_term + ki_term;
  
  // 角度转步数并限幅
  int32_t step_output = (int32_t)(angle_output / ANGLE_PER_STEP);
  step_output = Limit_Value(step_output, MAX_STEPS, MIN_STEPS);
  
  last_dx = dx;
  return step_output;
}

// --------------------------
// 步进电机PWM周期计算
// --------------------------
int Calculate_target(int Target) {
  if (Target == 0) return 0;
  
  int direction = (Target >= 0) ? 1 : -1;
  float speed = (float)abs(Target);
  speed = Limit_Value_Float(speed, MAX_SPEED, MIN_SPEED);
  
  // 转速转每秒步数
  float steps_per_rev = STEPS_PER_REVOLUTION * MICROSTEPS;
  float steps_per_second = (speed * steps_per_rev) / 60.0f;
  
  // 步数转PWM频率
  float pwm_freq = steps_per_second;
  pwm_freq = Limit_Value_Float(pwm_freq, 20000.0f, 500.0f);
  
  // 频率转周期值
  uint16_t period_value = (uint16_t)((PWM_CLOCK_FREQ / pwm_freq) - 1);
  period_value = Limit_Value(period_value, 65535, 1);
  
  return direction * period_value;
}

// --------------------------
// 步进电机驱动
// --------------------------
void Set_Bujin_PWM(int Target) {
  if (Target > 0) { // 正转
    DL_GPIO_setPins(GPIO_dizuo_PORT, GPIO_dizuo_EN_PIN);
    DL_GPIO_setPins(GPIO_dizuo_PORT, GPIO_dizuo_DIR_PIN);
    DL_TimerA_setLoadValue(PWM_dizuo_INST, abs(Target));
    DL_Timer_setCaptureCompareValue(PWM_dizuo_INST, abs(Target)/2, DL_TIMER_CC_0_INDEX);
  } else if (Target < 0) { // 反转
    DL_GPIO_setPins(GPIO_dizuo_PORT, GPIO_dizuo_EN_PIN);
    DL_GPIO_clearPins(GPIO_dizuo_PORT, GPIO_dizuo_DIR_PIN);
    DL_TimerA_setLoadValue(PWM_dizuo_INST, abs(Target));
    DL_Timer_setCaptureCompareValue(PWM_dizuo_INST, abs(Target)/2, DL_TIMER_CC_0_INDEX);
  } else { // 停止
    DL_GPIO_clearPins(GPIO_dizuo_PORT, GPIO_dizuo_EN_PIN);
  }
}

// --------------------------
// 步进电机加减速控制
// --------------------------
void yuntai_aim(float dx) {
  int32_t target_steps = pid_calculate(dx);
  
  if (target_steps == 0) {
    current_speed = 0;  // 快速停稳
    Set_Bujin_PWM(0);
    return;
  }
  
  // 分段限制最大速度
  int max_speed = (fabs(dx) <= SMALL_DX_THRESHOLD) ? MAX_SPEED_SMALL : MAX_SPEED;
  int target_speed = (int)(abs(target_steps) * 60.0f / (STEPS_PER_REVOLUTION * MICROSTEPS * 0.01f));
  target_speed = Limit_Value(target_speed, max_speed, MIN_SPEED);
  
  // 加减速控制
  int speed_dir = (target_steps > 0) ? 1 : -1;
  int target_speed_with_dir = speed_dir * target_speed;
  printf("target_speed_with_dir:%d",target_speed_with_dir);
  if (target_speed_with_dir > current_speed) {
    current_speed += MAX_ACCELERATION;
    if (current_speed > target_speed_with_dir) current_speed = target_speed_with_dir;
  } else {
    current_speed -= MAX_ACCELERATION;
    if (current_speed < target_speed_with_dir) current_speed = target_speed_with_dir;
  }
  int pwm_target = Calculate_target(current_speed);
  Set_Bujin_PWM(pwm_target);
  
  // 更新累计步数
  current_steps += target_steps;
  current_steps = Limit_Value(current_steps, MAX_STEPS * 10, MIN_STEPS * 10);
}


// --------------------------
// 舵机角度转占空比
// --------------------------
float angle_to_duty(float angle_error) {
  float target_duty = MID_DUTY + angle_error * DUTY_PER_DEG;
  return Limit_Value_Float(target_duty, MAX_DUTY, MIN_DUTY);
}
// --------------------------
// 舵机PID控制（含积分项）
// --------------------------
void biyao_pid_control(float angle_error) 
{
  // 比例项
  float kp_term = BIYAO_KP * angle_error;
  // 微分项
  float kd_term = BIYAO_KD * (angle_error - last_angle_error);
  integral_angle += angle_error * BIYAO_KI;
  // integral_angle = Limit_Value_Float(integral_angle, MAX_ANGLE_INTEGRAL, -MAX_ANGLE_INTEGRAL);
  // 总误差计算
  float total_error = kp_term + kd_term + integral_angle;
  // 设置占空比
  current_duty = angle_to_duty(total_error);
  biyao_Set_Angle(current_duty);
  last_angle_error = angle_error;
}

// --------------------------
// 舵机PWM设置
// --------------------------
void biyao_Set_Angle(float duty) 
{
  uint32_t compareValue = (uint32_t)(7999.0f - 7999.0f * (duty / 100.0f));
  DL_TimerA_setCaptureCompareValue(PWM_biyao_INST, compareValue, DL_TIMER_CC_0_INDEX);
}









