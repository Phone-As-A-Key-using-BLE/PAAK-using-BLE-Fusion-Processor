/***********************************************
 * Module Name: ADC Driver
 * Purpose: Provide APIs for ADC operations
 ***********************************************/

/***********************************************
 * Includes
 ***********************************************/
#include "adc.h"

/***********************************************
 * Local Variables
 ***********************************************/
static uint16_t adcSamples[ADC_MAX_SAMPLES];
static uint8_t sampleIndex = 0;

/***********************************************
 * Function Definitions
 ***********************************************/

/***********************************************
 * Function Name: ADC_Init
 * Purpose: Initialize ADC module
 ***********************************************/
void ADC_Init(void) {
    // Enable ADC0 and GPIOE peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    // Configure GPIO pin for ADC0 (PE3)
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    // Configure ADC sequencer
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 3);
}

/***********************************************
 * Function Name: ADC_ReadTemperature
 * Purpose: Read raw temperature value from ADC
 ***********************************************/
uint16_t ADC_ReadTemperature(void) {
    uint32_t adcValue;

    // Trigger ADC conversion
    ADCProcessorTrigger(ADC0_BASE, 3);

    // Wait for conversion to complete
    while (!ADCIntStatus(ADC0_BASE, 3, false));

    // Read ADC value
    ADCSequenceDataGet(ADC0_BASE, 3, &adcValue);
    ADCIntClear(ADC0_BASE, 3);

    return (uint16_t)adcValue;
}

/***********************************************
 * Function Name: ADC_GetAveragedTemperature
 * Purpose: Get averaged temperature reading
 ***********************************************/
uint16_t ADC_GetAveragedTemperature(void) {
    uint32_t sum = 0;
    uint8_t i;

    // Add current sample to array
    adcSamples[sampleIndex] = ADC_ReadTemperature();
    sampleIndex = (sampleIndex + 1) % ADC_MAX_SAMPLES;

    // Calculate average
    for (i = 0; i < ADC_MAX_SAMPLES; i++) {
        sum += adcSamples[i];
    }

    return (uint16_t)(sum / ADC_MAX_SAMPLES);
}
bool ADC_CheckTemperature(void) {
    uint16_t temp = ADC_ReadTemperature();

    // Simulate conversion to degrees Celsius
    uint16_t tempCelsius = (temp * 330) / 4095;  // Example conversion for a 3.3V reference

    return tempCelsius <= TEMP_THRESHOLD;
}
