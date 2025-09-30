/*
 * comparator.c
 *
 *  Created on: Sep. 26, 2020
 *      Author: Alka
 *      Modified by TempersLee Step. 26, 2025, porting for CH32L103
 */

#include "comparator.h"

#include "targets.h"


uint8_t getCompOutputLevel()
{
    return (GPIOA->INDR & GPIO_Pin_3);
}

void maskPhaseInterrupts()
{

    EXTI->INTENR &= ~(0x1 << 3); //mask
    EXTI->INTFR  = (0x1<<3);     //clear_flag
}

void enableCompInterrupts()
{
    EXTI->INTFR  = (1<<3);      //clear_flag
    EXTI->INTENR |= (1 << 3);   //enable interrupt
}

void changeCompInput()
{
    if(step == 1 || step == 4) // c floating
    {
        OPA->CTLR1 = (OPA_Trim|0x0021);
    }
    if(step == 2 || step == 5) // a floating
    {
        // OPA->CTLR1 = 0x70;
         OPA->CTLR1 = (OPA_Trim|0x0321);
    }
    if(step == 3 || step == 6) // b floating
    {
        // OPA->CTLR1 &= 0xFFFF0000;
        // OPA->CTLR1 = 0x30;
         OPA->CTLR1 = (OPA_Trim|0x0221);
    }
    // if (rising)
    // {
    //     EXTI->RTENR = 0;       //CC connect to OPA_P,Phase connect to OPA_N
    //     EXTI->FTENR = (1<<3);  //CC connect to OPA_P,Phase connect to OPA_N
    // }
    // else
    // {
    //     EXTI->FTENR = 0;       //CC connect to OPA_P,Phase connect to OPA_N 
    //     EXTI->RTENR = (1<<3);  //CC connect to OPA_P,Phase connect to OPA_N
    // }

    EXTI->FTENR = (rising) << 3;  
    EXTI->RTENR = (!rising) << 3;

}
