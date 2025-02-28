/***********************************************
 * Configuration File: ADC Driver
 * Purpose: Configure ADC parameters
 ***********************************************/

#ifndef ADC_CFG_H
#define ADC_CFG_H

/***********************************************
 * ADC Configuration
 ***********************************************/
#define ADC_BASE_ADDRESS      ADC0_BASE  // ADC base address
#define ADC_SEQUENCE_NUMBER   3          // ADC sequence number
#define ADC_INPUT_CHANNEL     ADC_CTL_CH0 // Input channel (e.g., CH0 for PE3)
#define ADC_MAX_VALUE         4095       // Maximum ADC value (12-bit resolution)

#endif /* ADC_CFG_H */
