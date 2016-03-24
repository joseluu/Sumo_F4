/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "tim.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */

#include "Control.h"
#undef FLASH_LATENCY_2
#define FLASH_LATENCY_2 FLASH_LATENCY_3 // does not work otherwise

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
/* Handle all EXTI lines */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    /* Check proper line */
	if (GPIO_Pin == B1_Pin){ // Blue push button: start
		do_startButton();
	} else if (GPIO_Pin == ECHO_11_A1_EXTI1_Pin){
		do_radarDetect(FRONT_LEFT_RADAR, __HAL_TIM_GET_COUNTER(&htim2));

	} else if (GPIO_Pin == ECHO_10_A2_EXTI4_Pin){
		do_radarDetect(FRONT_CENTER_RADAR, __HAL_TIM_GET_COUNTER(&htim2));

	} else if (GPIO_Pin == ECHO_12_A3_EXTI0_Pin){
		do_radarDetect(FRONT_RIGHT_RADAR, __HAL_TIM_GET_COUNTER(&htim2));

	} else if (GPIO_Pin == ECHO1_PC10_EXTI10_Pin){
		do_radarDetect(RIGHT_RADAR, __HAL_TIM_GET_COUNTER(&htim2));

	} else if (GPIO_Pin == ECHO2_PA15_EXTI15_Pin){
		do_radarDetect(LEFT_RADAR, __HAL_TIM_GET_COUNTER(&htim2));

	} else if (GPIO_Pin == ECHO3_PC12_EXTI12_Pin){
		do_radarDetect(BACK_RADAR, __HAL_TIM_GET_COUNTER(&htim2));

	} else if (GPIO_Pin == CNY1_Pin){
		do_frontEdgeDetect(RIGHT);

	} else if (GPIO_Pin == CNY2_Pin){
		do_frontEdgeDetect(LEFT); // 

	} else if (0 && (GPIO_Pin == ECHO_11_A1_EXTI1_Pin)){
		do_backEdgeDetect(LEFT);
	}
}


void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{

	if (htim == &htim10) {
		doWakeup(0);
	} else if (htim == &htim11) {
		doWakeup(1);
	} else if (htim == &htim13) {
		doWakeup(2);
	} else if (htim == &htim14) {
		doWakeup(3);
	} else {
		// error
	}
// signal something
		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET); // sync pulse on PA_0  (arduino A0)
		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
}

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  MX_TIM10_Init();
  MX_TIM11_Init();
  MX_TIM13_Init();
  MX_TIM14_Init();

  /* USER CODE BEGIN 2 */
	                       
	do_initializeControl();

	HAL_TIM_Base_Start(&htim5); // microsecond counter just in case
	HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_1); // ultrasound pulse firing
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1); // PWM1
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); // PWM2


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
   do_stepInLoop();
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  __PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  HAL_PWREx_ActivateOverDrive();

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
