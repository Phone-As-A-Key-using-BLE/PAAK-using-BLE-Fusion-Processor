#ifndef TIMER_DRIVER_H
#define TIMER_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

typedef void (*TimerCallback_t)(void);

void TimerDriver_Init(void);
void TimerDriver_Start(uint32_t milliseconds, TimerCallback_t callback);
void TimerDriver_Stop(void);

#endif // TIMER_DRIVER_H
