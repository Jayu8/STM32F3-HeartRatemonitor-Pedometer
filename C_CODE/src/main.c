/**
  ******************************************************************************
  * @file    main.c
  * @author  Ke Yang, Jiadong Chen
  * @version V1.0
  * @date    28-April-2017
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f3xx.h"
#include "stm32f3_discovery.h"

#include "cpu.h"
#include "board_led.h"
#include "uart.h"
//#include "stm32f3xx_ll_adc.h"



ADC_HandleTypeDef ADCHandle;
static ADC_ChannelConfTypeDef ADCChannel;
static int32_t ADC_Value;
static int32_t ADC_Ave_Value;
#define downSample 300
static int16_t XYZ_Value[3];
static int16_t X,Y,Z;
static int32_t X_Ave, Y_Ave, Z_Ave;
static int32_t acc_Ave;
static const int8_t check = 0x66;

static I2C_HandleTypeDef  hi2c;
static RCC_PeriphCLKInitTypeDef ADCLOCK;

void ADCPC1_Init()
{
	 GPIO_InitTypeDef  GPIO_InitStruct;

		GPIO_InitStruct.Pin = GPIO_PIN_1;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

		__HAL_RCC_GPIOC_CLK_ENABLE();

		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		__HAL_RCC_ADC12_CLK_ENABLE();
		ADCLOCK.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
		ADCLOCK.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV4;
		ADCLOCK.RTCClockSelection = RCC_PERIPHCLK_RTC;
		HAL_RCCEx_PeriphCLKConfig(&ADCLOCK);
//	__HAL_RCC_ADC12_CONFIG();
//	__ADC1_CLK_ENABLE();
//	__HAL_RCC_GPIOC_CLK_ENABLE();



	ADCHandle.Instance = ADC1;
	ADCHandle.Init.Resolution = ADC_RESOLUTION_12B;
//	ADCHandle.Instance->SMPR1 |= (4U << 0);	//Sampling time selection 19.5 ADC clock cycles
//	ADCHandle.Instance->SQR1 |= ADC_SQR1_SQ1_0;	//conversion in regular sequence channel number 1


//	HAL_ADC_MspDeInit(&ADCHandle);
//	HAL_ADC_DeInit(&ADCHandle);


	HAL_ADC_Init(&ADCHandle);



	ADCChannel.Channel = ADC_CHANNEL_7;
	ADCChannel.Rank = ADC_REGULAR_RANK_1;
	ADCChannel.SamplingTime = ADC_SAMPLETIME_19CYCLES_5;

	HAL_ADC_ConfigChannel(&ADCHandle, &ADCChannel);

}






void I2C_Init()
{

	 GPIO_InitTypeDef  GPIO_InitStruct;	//PB6 SCL, PB7 SDA

			GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
			GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

			__HAL_RCC_GPIOB_CLK_ENABLE();

			HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

			__HAL_RCC_I2C1_CLK_ENABLE();
			hi2c.Instance = I2C1;
//			hi2c.Init.OwnAddress1 = 0x19;//linear acceleration the default (factory) 7-bit slave address is 0011001b.
//			hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
			HAL_I2C_Init(&hi2c);


			LSM303DLHC_AccInit(0x0097);


}


void ADC_Conversion(){

	HAL_ADC_Start(&ADCHandle);
	HAL_ADC_PollForConversion(&ADCHandle, 0xFFFFFF);
	ADC_Value = HAL_ADC_GetValue(&ADCHandle);
	ADC_Ave_Value += ADC_Value;
}




void ACC()
{
	LSM303DLHC_AccReadXYZ(XYZ_Value);
	X = XYZ_Value[0]/128;
	Y = XYZ_Value[1]/128;
	Z = XYZ_Value[2]/128;
	X_Ave += X;
	Y_Ave += Y;
	Z_Ave += Z;
}


int main()
{
  /*
    Initialize the PLL, clock tree to all peripherals, flash and Systick 1 ms time reference:
   */
  cpu_init();
  /*
    Initialize the GPIO (General-Purpose I/O) subsystem pins that are connected to the LEDs on the board:
   */
  board_led_init();

  uart_debug_init();

  I2C_Init();

  ADCPC1_Init();

int i=0;


while(1){
  /*
    In an infinite loop, keep toggling the LEDs in an alternating pattern:
   */

if(i%downSample==0){

	ADC_Ave_Value = ADC_Ave_Value/downSample;
	X_Ave = X_Ave/downSample;
	Y_Ave = Y_Ave/downSample;
	Z_Ave = Z_Ave/downSample;
	acc_Ave = sqrt(X_Ave*X_Ave + Y_Ave*Y_Ave + Z_Ave*Z_Ave);

	uart_send_data_blocking(UART_PORT1, &check, 1U);
	uart_send_data_blocking(UART_PORT1, &acc_Ave, 2U);
	uart_send_data_blocking(UART_PORT1, &ADC_Ave_Value, 2U);

	ADC_Ave_Value = 0;
	X_Ave = 0;
	Y_Ave = 0;
	Z_Ave = 0;
}
//  uart_send_data_blocking(UART_PORT1, (uint8_t *)"Hello World!!\r\n", 15U);
//  uart_get_data_blocking(UART_PORT1,&abc,15U);
  ADC_Conversion();

  ACC();




 //     board_led_on(LED1);
 //     board_led_off(LED2);
 //     board_led_off(LED3);

 //     cpu_sw_delay(50U);  // Invoke a simple software busy-wait routine to delay approximately 50 milliseconds

 //     board_led_off(LED1);
 //     board_led_on(LED2);
 //     board_led_on(LED3);

 //     cpu_sw_delay(50U);

      ++i; // Increment i for the next test iteration...


}
  return 0;
}
