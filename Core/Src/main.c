/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "MQTTSim800.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define KEEP_ALIVE_INTERVAL 60
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
SIM800_t SIM800;
volatile uint8_t interrupt = 0;
uint32_t lastKeepAliveTime = 0;
extern uint8_t mqtt_receive;
extern uint8_t rx_data;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)
{
    if (huart == UART_SIM800)
    {
        Sim800_RxCallBack();
        if (SIM800.mqttServer.connect == 1 && rx_data == 0xD0) {
            printf("Received MQTT PINGRESP\n\r");
            mqtt_receive = 1;
        }
    }
}

uint32_t millis() {
    // HAL_GetTick() returns milliseconds since startup
    return HAL_GetTick();
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
//  HAL_GPIO_WritePin(SIM_PWR_GPIO_Port, SIM_PWR_Pin, GPIO_PIN_RESET);
//  HAL_Delay(3000);
//  HAL_GPIO_WritePin(SIM_PWR_GPIO_Port, SIM_PWR_Pin, GPIO_PIN_SET);
//  HAL_Delay(10000);


  SIM800.sim.apn = "internet";
  SIM800.sim.apn_user = "";
  SIM800.sim.apn_pass = "";
  SIM800.mqttServer.host = "mqtt.onwords.in";
  SIM800.mqttServer.port = 1883;
  SIM800.mqttClient.username = "Nikhil";
  SIM800.mqttClient.pass = "Nikhil8182";
  SIM800.mqttClient.clientID = "TestSub";
  SIM800.mqttClient.keepAliveInterval = 60;
  MQTT_Init();
  uint8_t sub = 0;
  uint8_t pub_uint8 = 1;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (SIM800.mqttServer.connect == 0)
	  {
		   MQTT_Init();
		   sub = 0;
		   lastKeepAliveTime = millis();
	   }
	   if (SIM800.mqttServer.connect == 1)
	   {
		   if(sub == 0){
			   MQTT_Sub("STM32/subscribe");
			   sub = 1;
		   }
		   if(interrupt)
		   {
			   HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
			   MQTT_Pub("STM32/publish", "LED Toggled");
			   printf("data published\n\r");
			   interrupt = 0;
			   lastKeepAliveTime = millis();
		   }
		   if(SIM800.mqttReceive.newEvent) {
			   printf("data received\n\r");
			   unsigned char *topic = SIM800.mqttReceive.topic;
			   unsigned char *payload = SIM800.mqttReceive.payload;

			   // Print topic and payload
			   printf("Topic: %s\n\r", topic);
			   printf("Payload: %s\n\r", payload);
			   if(strcmp((char *)payload, "LED") == 0)
			   {
				   HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
				   MQTT_Pub("STM32/publish", "LED Toggled");

			   }
//			   unsigned char *topic = SIM800.mqttReceive.topic;
//			   int payload = atoi(SIM800.mqttReceive.payload);
			  /// printf("%s", payload);
			   SIM800.mqttReceive.newEvent = 0;
		   }
		   if ((millis() - lastKeepAliveTime) >= (KEEP_ALIVE_INTERVAL * 100)) {
			   MQTT_PubUint8("STM32/pingreq", pub_uint8);
			   printf("Sending MQTT PINGREQ\n\r");
			   lastKeepAliveTime = millis();
		   }
	   }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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

/* USER CODE BEGIN 4 */

#ifdef __GNUC__
#define UART_printf   int __io_putchar(int ch)
UART_printf
{
	HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
	return ch;
}
#endif

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == USER_BT_Pin)
	{
		interrupt = 1;
		printf("interrupt triggered\n\r");
	}
}
/* USER CODE END 4 */

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
