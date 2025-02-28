#ifndef SYSTICKTIMER_CFG_H
#define SYSTICKTIMER_CFG_H

/***********************************************
 * Configuration Parameters for SysTick Timer
 ***********************************************/

/* Define the time intervals in milliseconds */
#define SYSTICK_INTERVAL_1MS   1000U   // For 1 ms interval (1000 Hz)
#define SYSTICK_INTERVAL_5MS    200U   // For 5 ms interval (200 Hz)
#define SYSTICK_INTERVAL_10MS   100U   // For 10 ms interval (100 Hz)
#define SYSTICK_INTERVAL_50MS    20U   // For 50 ms interval (20 Hz)
#define SYSTICK_INTERVAL_100MS   10U   // For 100 ms interval (10 Hz)

/* Select the desired interval by defining one of the above options */
#define SYSTICK_TIME_INTERVAL   SYSTICK_INTERVAL_50MS

#endif // SYSTICKTIMER_CFG_H
