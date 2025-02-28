/***********************************************
 * Module Name: SYSTICKTIMER
 * Author: ABDULLAH
 * Purpose: Initialize systick timer & handler
 ***********************************************/

#ifndef SYSTICKTIMER_H
#define SYSTICKTIMER_H


/***********************************************
 * Includes
 ***********************************************/
#include <stdint.h>
#include <stdbool.h>

#include "systicktimer_cfg.h"
/***********************************************
 * Function Prototypes
 ***********************************************/

//void SysTick_Init(void);
void SysTick_Handler(void);
void SysTick_Init(void);

#endif /* SYSTICKTIMER_H */
