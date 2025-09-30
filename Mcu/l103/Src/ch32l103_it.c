/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32l103_it.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/10/30
 * Description        : Main Interrupt Service Routines.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "ch32l103_it.h"
#include "main.h"
#include "ADC.h"
#include "targets.h"
#include "IO.h"
#include "common.h"
#include "comparator.h"

extern void transfercomplete();
extern void PeriodElapsedCallback();
extern void interruptRoutine();
extern void tenKhzRoutine();
extern void processDshot();

extern char send_telemetry;
uint16_t interrupt_time = 0;
extern char servoPwm;
extern char dshot_telemetry;
extern char armed;
extern char out_put;
extern uint8_t compute_dshot_flag;
extern uint32_t commutation_interval;


void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

//for adc DMA
void DMA1_Channel1_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
//for IC timer DMA
void DMA1_Channel5_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
//for 20KHz int
void LPTIM_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
//for COMP int
void EXTI3_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
//for tele DMA
void DMA1_Channel7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
//for com
void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
//dor dshot
void SW_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));


/*********************************************************************
 * @fn      NMI_Handler
 *
 * @brief   This function handles NMI exception.
 *
 * @return  none
 */
void NMI_Handler(void)
{
    while (1)
    {
    }
}


/*********************************************************************
 * @fn      HardFault_Handler
 *
 * @brief   This function handles Hard Fault exception.
 *
 * @return  none
 */
void HardFault_Handler(void)
{
    NVIC_SystemReset();
    while (1)
    {
    }
}

// for adc
void DMA1_Channel1_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC1))
    {
        DMA_ClearFlag(DMA1_IT_TC1|DMA1_IT_HT1);
        ADC_DMA_Callback( );
    }
    /* Check whether DMA transfer error caused the DMA interruption */
    if(DMA_GetITStatus(DMA1_IT_TE1))
    {
        DMA_ClearFlag(DMA1_IT_TE1);
    }

}

//for ic timer
void DMA1_Channel5_IRQHandler(void)
{
    if(DMA1->INTFR & DMA1_IT_HT5) 
    {
        if(servoPwm)
        {
            IC_TIMER_REGISTER->CCER = 0x03;  
        }
        DMA1->INTFCR = DMA1_IT_HT5;
    }
    if( DMA1->INTFR & DMA1_IT_TC5)
    {
        CLEAR_BIT(INPUT_DMA_CHANNEL->CFGR,0x1);  //disable DMA1_CH5
        transfercomplete();
        DMA1->INTFCR = DMA1_IT_TC5;
        // input_ready = 1;
        NVIC_SetPendingIRQ(Software_IRQn);
    }
    /* Check whether DMA transfer error caused the DMA interruption */
    if( DMA1->INTFR & DMA1_IT_TE5)
    {
        CLEAR_BIT(INPUT_DMA_CHANNEL->CFGR,0x1);  //disable DMA1_CH5
        transfercomplete( );
        DMA_ClearFlag(DMA1_IT_TE5);
        // input_ready = 1;
         NVIC_SetPendingIRQ(Software_IRQn);
    }
}

void LPTIM_IRQHandler(void)
{
    if(LPTIM->ISR & LPTIM_FLAG_ARRM)
    {
        LPTIM->ICR |= (uint32_t)LPTIM_FLAG_ARRM;
        tenKhzRoutine( );
    }

}

void EXTI3_IRQHandler(void)
{
    if((INTERVAL_TIMER->CNT) > ((average_interval>>1))){
        EXTI->INTFR = EXTI_Line3;
        interruptRoutine();
     }else{ 
       if (getCompOutputLevel() == rising){
        EXTI->INTFR = EXTI_Line3;
     }
   }
}


//for tele
void DMA1_Channel7_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC7))
    {
        USART_DMACmd(USART2,USART_DMAReq_Tx,DISABLE);
        DMA_Cmd(DMA1_Channel7, DISABLE);
        MODIFY_REG(USART2->CTLR1, 0x3<<2, 0x0<<3);  //disable send
        DMA_ClearFlag(DMA1_IT_TC7);
    }
    /* Check whether DMA transfer error caused the DMA interruption */
    if(DMA_GetITStatus(DMA1_IT_TE7))
    {
        USART_DMACmd(USART2,USART_DMAReq_Tx,DISABLE);
        MODIFY_REG(USART2->CTLR1, 0x3<<2, 0x0<<3);  //disable send
        DMA_Cmd(DMA1_Channel7, DISABLE);
        DMA_ClearFlag(DMA1_IT_TE7);
    }
}

//for com
void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3,TIM_IT_Update))
    {
        PeriodElapsedCallback( );
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}


//for processDshot
void SW_Handler(void)
{
    interrupt_time++;
    processDshot( );
}

