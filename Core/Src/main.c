/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Private includes ----------------------------------------------------------*/


/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* Private define ------------------------------------------------------------*/
#define DIR_PIN GPIO_PIN_3
#define DIR_PORT GPIOA
#define STEP_PIN GPIO_PIN_4
#define STEP_PORT GPIOA
#define ENABLED_PIN GPIO_PIN_5
#define ENABLED_PORT GPIOA

#define SERVO_PIN GPIO_PIN_1
#define SERVO_PORT GPIOA

#define TX_PIN GPIO_PIN_9
#define TX_PORT GPIOA
#define RX_PIN GPIO_PIN_10
#define RX_PORT GPIOA

#define RELAY_PIN GPIO_PIN_11
#define RELAY_PORT GPIOB

#define IR_PIN GPIO_PIN_9
#define IR_PORT GPIOB

#define SWITCH_PIN GPIO_PIN_0
#define SWITCH_PORT GPIOB

#define TRIG_PIN_1 GPIO_PIN_5
#define TRIG_PORT_1 GPIOB
#define ECHO_PIN_1 GPIO_PIN_7
#define ECHO_PORT_1 GPIOB

#define TRIG_PIN_2 GPIO_PIN_3
#define TRIG_PORT_2 GPIOB
#define ECHO_PIN_2 GPIO_PIN_4
#define ECHO_PORT_2 GPIOB

#define TRIG_PIN_3 GPIO_PIN_14
#define TRIG_PORT_3 GPIOB
#define ECHO_PIN_3 GPIO_PIN_15
#define ECHO_PORT_3 GPIOB

#define TRIG_PIN_4 GPIO_PIN_11
#define TRIG_PORT_4 GPIOA
#define ECHO_PIN_4 GPIO_PIN_13
#define ECHO_PORT_4 GPIOB

/* Private variables ---------------------------------------------------------*/
uint32_t pMillis;
uint32_t Value1 = 0;
uint64_t Value2 = 0;
uint64_t Value3 = 0;
uint16_t Distance1 = 0; 
uint16_t Distance2 = 0; 
uint16_t Distance3 = 0; 
uint8_t d = 0;
uint8_t Rx_data[10];
uint8_t rx_data;
uint8_t tx_data_1[3]="1\r\n";
uint8_t tx_data_2[3]="2\r\n";
uint8_t tx_data_3[3]="3\r\n";
/* USER CODE END PTD */


/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/


/* USER CODE BEGIN PV */
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
int stepDelay = 1000; // 1000us more delay means less speed
int32_t ctht = 0;

void microDelay (uint16_t delay)
{
  __HAL_TIM_SET_COUNTER(&htim1, 0);
  while (__HAL_TIM_GET_COUNTER(&htim1) < delay);
}

uint16_t distance(GPIO_TypeDef *trig_port, uint16_t GPIO_Pin_trig, GPIO_TypeDef *echo_port, uint16_t GPIO_Pin_echo)
{
	uint16_t Distance = 0;
	uint32_t Value1 = 0;
	uint64_t Value2 = 0;
	HAL_GPIO_WritePin(trig_port, GPIO_Pin_trig, GPIO_PIN_SET);  // pull the TRIG pin HIGH
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	while (__HAL_TIM_GET_COUNTER (&htim1) < 10); // wait for 10 us
	HAL_GPIO_WritePin(trig_port, GPIO_Pin_trig, GPIO_PIN_RESET);  // pull the TRIG pin low
	pMillis = HAL_GetTick(); // used this to avoid infinite while loop  (for timeout)
	// wait for the echo pin to go high
	while(!(HAL_GPIO_ReadPin(echo_port, GPIO_Pin_echo)) && pMillis+100>HAL_GetTick());
	Value1 = __HAL_TIM_GET_COUNTER(&htim1);
	//HAL_Delay(2);
	pMillis = HAL_GetTick(); // used this to avoid infinite while loop (for timeout)
	// wait for the echo pin to go low
	while((HAL_GPIO_ReadPin(echo_port, GPIO_Pin_echo)) && pMillis+500>HAL_GetTick());
	Value2 = __HAL_TIM_GET_COUNTER(&htim1);
	Distance = (Value2-Value1)* 0.034/2;
	return Distance;
}
void to_camera(){
	HAL_GPIO_WritePin(ENABLED_PORT, ENABLED_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DIR_PORT, DIR_PIN, GPIO_PIN_RESET);//Clock wise rotation
	for(int i=1;i<=2000;i++){  //Moving stepper motor forward
		HAL_GPIO_WritePin(STEP_PORT, STEP_PIN, GPIO_PIN_SET);
		microDelay(1000);
		HAL_GPIO_WritePin(STEP_PORT, STEP_PIN, GPIO_PIN_RESET);
		microDelay(1000);
		if(!(HAL_GPIO_ReadPin(IR_PORT, IR_PIN))){
			HAL_GPIO_WritePin(ENABLED_PORT, ENABLED_PIN, GPIO_PIN_SET);
			break;
		}
	}
	HAL_GPIO_WritePin(ENABLED_PORT, ENABLED_PIN, GPIO_PIN_SET);
}
void move(char position, uint8_t dir)
{
	uint16_t pulse = 0;
	HAL_GPIO_WritePin(ENABLED_PORT, ENABLED_PIN, GPIO_PIN_RESET);
	if(dir == 1){
		if(position == '1'){
			pulse = 1620;
			HAL_GPIO_WritePin(DIR_PORT, DIR_PIN, GPIO_PIN_SET);
		}
		else if(position == '2'){
			pulse = 400;
			HAL_GPIO_WritePin(DIR_PORT, DIR_PIN, GPIO_PIN_SET);
		}
		else if(position == '3'){
			pulse = 820;
			HAL_GPIO_WritePin(DIR_PORT, DIR_PIN, GPIO_PIN_RESET);
		}
	}
	else if(dir == 0){
		if(position == '1'){
			pulse = 650;
			HAL_GPIO_WritePin(DIR_PORT, DIR_PIN, GPIO_PIN_RESET);
		}
		else if(position == '2'){
			pulse = 600;
			HAL_GPIO_WritePin(DIR_PORT, DIR_PIN, GPIO_PIN_SET);
		}
		else if(position == '3'){
			pulse = 1770;
			HAL_GPIO_WritePin(DIR_PORT, DIR_PIN, GPIO_PIN_SET);
		}
	}
	for(int i=1;i<=pulse;i++){  //Moving stepper motor forward
		HAL_GPIO_WritePin(STEP_PORT, STEP_PIN, GPIO_PIN_SET);
		microDelay(1000);
		HAL_GPIO_WritePin(STEP_PORT, STEP_PIN, GPIO_PIN_RESET);
		microDelay(1000);
	}
	HAL_GPIO_WritePin(ENABLED_PORT, ENABLED_PIN, GPIO_PIN_SET);
	
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  HAL_Init();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
	HAL_TIM_Base_Start(&htim1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		HAL_GPIO_WritePin(ENABLED_PORT, ENABLED_PIN, GPIO_PIN_SET);
		if(HAL_GPIO_ReadPin(SWITCH_PORT, SWITCH_PIN)){
			while (1){
				Distance1 = distance(TRIG_PORT_4, TRIG_PIN_4, ECHO_PORT_4, ECHO_PIN_4);
				if(Distance1 < 25){
					HAL_UART_Transmit(&huart1,tx_data_1, sizeof(tx_data_1), 100);
					to_camera();
					
					HAL_UART_Transmit(&huart1,tx_data_2, sizeof(tx_data_2), 100);
					HAL_UART_Receive (&huart1, Rx_data, 3, 5000);
					while (Rx_data[0] != '2' && Rx_data[0] != '1' && Rx_data[0] != '3'){
						ctht++;
						//HAL_UART_Transmit(&huart1,tx_data, sizeof(tx_data), 100);
						HAL_UART_Receive (&huart1, Rx_data, 3, 5000);
					}
					d = 1;
					move(Rx_data[0], d);
					
					htim2.Instance->CCR2 = 350;
					HAL_Delay(1000);
					htim2.Instance->CCR2 = 2600;
					HAL_Delay(1000);
					
					d = 0;
					move(Rx_data[0], d);
					
					HAL_UART_Transmit(&huart1,tx_data_3, sizeof(tx_data_3), 100);
					break;
				}
			}
		}
  }
  /* USER CODE END WHILE */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  /* USER CODE END TIM1_Init 0 */

  /* USER CODE BEGIN TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 71;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE END TIM1_Init 1 */
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  /* USER CODE END TIM2_Init 0 */

  /* USER CODE BEGIN TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 19999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE END TIM2_Init 1 */

  /* USER CODE BEGIN TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);
  /* USER CODE END TIM2_Init 2 */
 
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE END USART1_Init 0 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_14|GPIO_PIN_3
                          |GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA3 PA4 PA5 PA11 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB11 PB14 PB3
                           PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_14|GPIO_PIN_3
                          |GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB13 PB15 PB4 PB7
                           PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_15|GPIO_PIN_4|GPIO_PIN_7
                          |GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
