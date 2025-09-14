/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define GPIO_HFXT_PORT                                                     GPIOA
#define GPIO_HFXIN_PIN                                             DL_GPIO_PIN_5
#define GPIO_HFXIN_IOMUX                                         (IOMUX_PINCM10)
#define GPIO_HFXOUT_PIN                                            DL_GPIO_PIN_6
#define GPIO_HFXOUT_IOMUX                                        (IOMUX_PINCM11)
#define CPUCLK_FREQ                                                     80000000



/* Defines for PWM_dizuo */
#define PWM_dizuo_INST                                                     TIMG6
#define PWM_dizuo_INST_IRQHandler                               TIMG6_IRQHandler
#define PWM_dizuo_INST_INT_IRQN                                 (TIMG6_INT_IRQn)
#define PWM_dizuo_INST_CLK_FREQ                                           800000
/* GPIO defines for channel 0 */
#define GPIO_PWM_dizuo_C0_PORT                                             GPIOB
#define GPIO_PWM_dizuo_C0_PIN                                      DL_GPIO_PIN_6
#define GPIO_PWM_dizuo_C0_IOMUX                                  (IOMUX_PINCM23)
#define GPIO_PWM_dizuo_C0_IOMUX_FUNC                 IOMUX_PINCM23_PF_TIMG6_CCP0
#define GPIO_PWM_dizuo_C0_IDX                                DL_TIMER_CC_0_INDEX

/* Defines for PWM_biyao */
#define PWM_biyao_INST                                                     TIMG7
#define PWM_biyao_INST_IRQHandler                               TIMG7_IRQHandler
#define PWM_biyao_INST_INT_IRQN                                 (TIMG7_INT_IRQn)
#define PWM_biyao_INST_CLK_FREQ                                           400000
/* GPIO defines for channel 0 */
#define GPIO_PWM_biyao_C0_PORT                                             GPIOB
#define GPIO_PWM_biyao_C0_PIN                                     DL_GPIO_PIN_15
#define GPIO_PWM_biyao_C0_IOMUX                                  (IOMUX_PINCM32)
#define GPIO_PWM_biyao_C0_IOMUX_FUNC                 IOMUX_PINCM32_PF_TIMG7_CCP0
#define GPIO_PWM_biyao_C0_IDX                                DL_TIMER_CC_0_INDEX



/* Defines for TIMER_0 */
#define TIMER_0_INST                                                     (TIMG0)
#define TIMER_0_INST_IRQHandler                                 TIMG0_IRQHandler
#define TIMER_0_INST_INT_IRQN                                   (TIMG0_INT_IRQn)
#define TIMER_0_INST_LOAD_VALUE                                          (1999U)



/* Defines for UART_WIT */
#define UART_WIT_INST                                                      UART3
#define UART_WIT_INST_FREQUENCY                                         80000000
#define UART_WIT_INST_IRQHandler                                UART3_IRQHandler
#define UART_WIT_INST_INT_IRQN                                    UART3_INT_IRQn
#define GPIO_UART_WIT_RX_PORT                                              GPIOA
#define GPIO_UART_WIT_RX_PIN                                      DL_GPIO_PIN_13
#define GPIO_UART_WIT_IOMUX_RX                                   (IOMUX_PINCM35)
#define GPIO_UART_WIT_IOMUX_RX_FUNC                    IOMUX_PINCM35_PF_UART3_RX
#define UART_WIT_BAUD_RATE                                              (115200)
#define UART_WIT_IBRD_80_MHZ_115200_BAUD                                    (43)
#define UART_WIT_FBRD_80_MHZ_115200_BAUD                                    (26)
/* Defines for UART_K230 */
#define UART_K230_INST                                                     UART2
#define UART_K230_INST_FREQUENCY                                        40000000
#define UART_K230_INST_IRQHandler                               UART2_IRQHandler
#define UART_K230_INST_INT_IRQN                                   UART2_INT_IRQn
#define GPIO_UART_K230_RX_PORT                                             GPIOB
#define GPIO_UART_K230_TX_PORT                                             GPIOB
#define GPIO_UART_K230_RX_PIN                                     DL_GPIO_PIN_16
#define GPIO_UART_K230_TX_PIN                                     DL_GPIO_PIN_17
#define GPIO_UART_K230_IOMUX_RX                                  (IOMUX_PINCM33)
#define GPIO_UART_K230_IOMUX_TX                                  (IOMUX_PINCM43)
#define GPIO_UART_K230_IOMUX_RX_FUNC                   IOMUX_PINCM33_PF_UART2_RX
#define GPIO_UART_K230_IOMUX_TX_FUNC                   IOMUX_PINCM43_PF_UART2_TX
#define UART_K230_BAUD_RATE                                               (9600)
#define UART_K230_IBRD_40_MHZ_9600_BAUD                                    (260)
#define UART_K230_FBRD_40_MHZ_9600_BAUD                                     (27)





/* Defines for DMA_WIT */
#define DMA_WIT_CHAN_ID                                                      (0)
#define UART_WIT_INST_DMA_TRIGGER                            (DMA_UART3_RX_TRIG)


/* Port definition for Pin Group GPIO_LED */
#define GPIO_LED_PORT                                                    (GPIOA)

/* Defines for PIN_LED: GPIOA.0 with pinCMx 1 on package pin 33 */
#define GPIO_LED_PIN_LED_PIN                                     (DL_GPIO_PIN_0)
#define GPIO_LED_PIN_LED_IOMUX                                    (IOMUX_PINCM1)
/* Port definition for Pin Group GPIO_BlueLight */
#define GPIO_BlueLight_PORT                                              (GPIOB)

/* Defines for PIN_BlueLight: GPIOB.4 with pinCMx 17 on package pin 52 */
#define GPIO_BlueLight_PIN_BlueLight_PIN                         (DL_GPIO_PIN_4)
#define GPIO_BlueLight_PIN_BlueLight_IOMUX                       (IOMUX_PINCM17)
/* Defines for PIN_S2: GPIOB.21 with pinCMx 49 on package pin 20 */
#define GPIO_BUTTON_PIN_S2_PORT                                          (GPIOB)
#define GPIO_BUTTON_PIN_S2_PIN                                  (DL_GPIO_PIN_21)
#define GPIO_BUTTON_PIN_S2_IOMUX                                 (IOMUX_PINCM49)
/* Defines for PIN_S1: GPIOA.18 with pinCMx 40 on package pin 11 */
#define GPIO_BUTTON_PIN_S1_PORT                                          (GPIOA)
#define GPIO_BUTTON_PIN_S1_PIN                                  (DL_GPIO_PIN_18)
#define GPIO_BUTTON_PIN_S1_IOMUX                                 (IOMUX_PINCM40)
/* Port definition for Pin Group GPIO_dizuo */
#define GPIO_dizuo_PORT                                                  (GPIOB)

/* Defines for EN: GPIOB.8 with pinCMx 25 on package pin 60 */
#define GPIO_dizuo_EN_PIN                                        (DL_GPIO_PIN_8)
#define GPIO_dizuo_EN_IOMUX                                      (IOMUX_PINCM25)
/* Defines for DIR: GPIOB.7 with pinCMx 24 on package pin 59 */
#define GPIO_dizuo_DIR_PIN                                       (DL_GPIO_PIN_7)
#define GPIO_dizuo_DIR_IOMUX                                     (IOMUX_PINCM24)



/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_PWM_dizuo_init(void);
void SYSCFG_DL_PWM_biyao_init(void);
void SYSCFG_DL_TIMER_0_init(void);
void SYSCFG_DL_UART_WIT_init(void);
void SYSCFG_DL_UART_K230_init(void);
void SYSCFG_DL_DMA_init(void);

void SYSCFG_DL_SYSTICK_init(void);

bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
