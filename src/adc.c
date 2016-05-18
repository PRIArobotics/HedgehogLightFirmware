#include "adc.h"
#include "gpio.h"
#include <stm32f4xx.h>


//0-7 = analog inputs, 8 = battery voltage
static gpio_pin_t analogPin[9] = {
	{GPIOA, 1}, //connected to PD0
	{GPIOA, 2}, //connected to PD1
	{GPIOA, 3}, //connected to PD2
	{GPIOA, 4}, //connected to PD3
	{GPIOA, 5}, //connected to PD4
	{GPIOA, 6}, //connected to PD5
	{GPIOA, 7}, //connected to PD6
	{GPIOB, 0}, //connected to PD7
	{GPIOB, 1}};

static uint16_t analogData[9] = {0,0,0,0,0,0,0,0,0};


void adc_init()
{
	//all adc pins to analog mode
	uint8_t i;
	for(i=0; i<9; i++) gpio_pinCfg(analogPin[i], MODE_ANA, 0);

	//ADC1 init
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; //enable clock for ADC1 (84MHz)
	ADC->CCR |= ADC_CCR_ADCPRE; //prescaler = 8
	ADC1->CR1 &= ~ADC_CR1_RES; //12bit resolution
	ADC1->CR1 |= ADC_CR1_SCAN; //scan mode enabled
	ADC1->CR2 |= ADC_CR2_EOCS; //EOC bit is set at the end of each regular conversion
	ADC1->CR2 |= ADC_CR2_DDS; //DMA requests are issued as long as data are converted and DMA=1
	ADC1->CR2 |= ADC_CR2_DMA; //DMA mode enabled
	ADC1->CR2 |= ADC_CR2_CONT; //continuous mode anabled

	//TODO: sample time registers

	//regular sequence, channel 1-9
	ADC1->SQR1 = (8<<20); //9 conversions in sequence
	ADC1->SQR2 = ((9<<10) | (8<<5) | (7<<0));
	ADC1->SQR3 = ((6<<25) | (5<<20) | (4<<15) | (3<<10) | (2<<5) | (1<<0));

	ADC1->CR2 |= ADC_CR2_ADON; //adc enabled

	//DMA2 stream 0 init
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN; //DMA2 clock enabled
	DMA2->LIFCR |= 0b111101; //clear all stream 0 interrupts
	DMA2_Stream0->PAR = (uint32_t)&ADC1->DR; //adc1 data register as peripheral source
	DMA2_Stream0->M0AR = (uint32_t)analogData; //data array as memory destination
	DMA2_Stream0->NDTR = 9; //9 data items
	DMA2_Stream0->CR &= ~DMA_SxCR_CHSEL; //channel 0
	DMA2_Stream0->CR |= DMA_SxCR_PL; //highest priority
	DMA2_Stream0->CR |= DMA_SxCR_MSIZE_0; //16bit memory data size
	DMA2_Stream0->CR |= DMA_SxCR_PSIZE_0; //16bit peripheral data size
	DMA2_Stream0->CR |= DMA_SxCR_MINC; //memory address is incremented by 2bytes after each transfer
	DMA2_Stream0->CR |= DMA_SxCR_CIRC; //circular mode
	DMA2_Stream0->CR |= DMA_SxCR_EN; //stream enabled

	ADC1->CR2 |= ADC_CR2_SWSTART; //start adc conversion
}


uint16_t adc_getAnalogInput(uint8_t input)
{
	if(input > 7) return 0;
	return analogData[input];
}


float adc_getBatteryVoltage()
{
	return (float)(analogData[8] * MAX_BATTERY_VOLTAGE / 4095);
}

uint16_t adc_getBatteryVoltage_mV()
{
	return (uint16_t)(analogData[8] * (MAX_BATTERY_VOLTAGE * 1000) / 4095);
}
