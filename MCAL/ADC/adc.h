/***********************************************
 * Module Name: ADC Driver
 * Purpose: Provide APIs for ADC operations
 ***********************************************/

#ifndef ADC_H
#define ADC_H

/***********************************************
 * Includes
 ***********************************************/
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/adc.h"
/***********************************************
 * ADC Configuration
 ***********************************************/
#define ADC_CHANNEL           0        // ADC channel for temperature sensor
#define ADC_SAMPLING_RATE_MS  500      // Sampling rate in milliseconds
#define ADC_MAX_SAMPLES       10       // Number of samples for averaging
#define ADC0_BASE               0x40038000  // ADC0

#define TEMP_THRESHOLD 70  // Overheat threshold in degrees Celsius


/***********************************************
 * Function Prototypes
 ***********************************************/
void ADC_Init(void);                           // Initialize ADC module
uint16_t ADC_ReadTemperature(void);            // Read raw temperature value
uint16_t ADC_GetAveragedTemperature(void);     // Get averaged temperature reading
bool ADC_CheckTemperature(void);

#endif /* ADC_H */
