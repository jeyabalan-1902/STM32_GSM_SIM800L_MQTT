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
#include "dma.h"
#include "rtc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "MQTTSim800.h"


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
TaskHandle_t mqttTaskHandle;
TaskHandle_t gpioTaskHandle;
TaskHandle_t uartTaskHandle;
TaskHandle_t rtcTaskHandle;

SemaphoreHandle_t xButtonSemaphore;

QueueHandle_t mqttQueue;
QueueHandle_t GSM_rxCallbackQueue;
QueueHandle_t ESP_uartQueue;

char timeData[15];
char dateData[15];

volatile uint8_t buttonState = 0;
SIM800_t SIM800;
uint32_t lastKeepAliveTime = 0;
extern uint8_t mqtt_receive;
extern uint8_t rx_data;

uint8_t uartRxBuffer[1];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void GSM_MQTT_Task(void *pvParameters);
void INT_GPIO_Task(void *pvParameters);
void ESP_UART_Task(void *pvParameters);
void RTC_Task(void *pvParameters);

uint32_t millis();
void set_time (uint8_t hr, uint8_t min, uint8_t sec);
void set_date (uint8_t year, uint8_t month, uint8_t date, uint8_t day);
void set_alarm (uint8_t hr, uint8_t min, uint8_t sec, uint8_t date);
void get_time_date(char *time, char *date);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void GSM_init(void)                                 //GSM Init
{
	SIM800.sim.apn = "internet";
	SIM800.sim.apn_user = "";
	SIM800.sim.apn_pass = "";
	SIM800.mqttServer.host = "";
	SIM800.mqttServer.port = 1883;
	SIM800.mqttClient.username = "";
	SIM800.mqttClient.pass = "";
	SIM800.mqttClient.clientID = "simTest";
	SIM800.mqttClient.keepAliveInterval = 60;
}

void FreeRTOS_Init(void)                            //FreeRTOS initialization function
{
    mqttQueue = xQueueCreate(5, sizeof(uint8_t));                 // Queue init
    ESP_uartQueue = xQueueCreate(10, sizeof(uartRxBuffer));

    xButtonSemaphore = xSemaphoreCreateBinary();          //binary semaphore init

    xTaskCreate(ESP_UART_Task, "ESP_UART_Task", 200, NULL, 3, &uartTaskHandle);
    xTaskCreate(RTC_Task, "RTC_Task", 200, NULL, 1, &rtcTaskHandle);
    xTaskCreate(GSM_MQTT_Task, "GSM_MQTT_Task", 512, NULL, 2, &mqttTaskHandle);
    xTaskCreate(INT_GPIO_Task, "INT_GPIO_Task", 256, NULL, 4, &gpioTaskHandle);
    vTaskStartScheduler();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)          // UART interrupt call back
{
	if (huart->Instance == UART4)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xQueueSendFromISR(ESP_uartQueue, uartRxBuffer, &xHigherPriorityTaskWoken);
		memset(uartRxBuffer, 0, sizeof(uartRxBuffer));
		HAL_UART_Receive_IT(&huart4, uartRxBuffer, sizeof(uartRxBuffer));
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}

    if (huart == UART_SIM800)
    {
    	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        Sim800_RxCallBack();
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void ESP_UART_Task(void *pvParameters)                             //UART task handler
{
    while (1) {
        if (xQueueReceive(ESP_uartQueue, uartRxBuffer, portMAX_DELAY) == pdTRUE) {
            printf("UART Data Received: %s\n", uartRxBuffer);
            if(uartRxBuffer == 1)
            {
            	printf("LED Toggled\n\r");
            	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            }
        }
    }
}

void INT_GPIO_Task(void *pvParameters)                             //GPIO Task Handler
{
    while (1)
    {
    	if(xSemaphoreTake(xButtonSemaphore, portMAX_DELAY) == pdTRUE)
    	{
    		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    		buttonState ^= 1;
			printf("button state toggled: %d\n\r", buttonState);
			xQueueSend(mqttQueue, (void *)&buttonState, portMAX_DELAY);
    	}
    }
}

void GSM_MQTT_Task(void *pvParameters)                             //GSM & MQTT Task Handler
{
   uint8_t receivedState;
   char mqttPayload[32];
   while(1)
   {
	   if (SIM800.mqttServer.connect == 0)
	   {
		   MQTT_Init();
		   lastKeepAliveTime = millis();
	   }
	  if (SIM800.mqttServer.connect == 1)
	   {
		  MQTT_Sub("STM32/subscribe");
		  if(SIM800.mqttReceive.newEvent)
		  {
			  unsigned char *topic = SIM800.mqttReceive.topic;
			  unsigned char *payload = SIM800.mqttReceive.payload;
			  SIM800.mqttReceive.newEvent = 0;
			  printf("Topic: %s\n\r", topic);
			  printf("Payload: %s\n\r",payload);
			  if(strcmp((char *)payload, "LED") == 0)
				{
					buttonState ^= 1;
					char mqttPayload[32];
					HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
					sprintf(mqttPayload, "{\"device1\":%d}", buttonState);
					MQTT_Pub("STM32/publish", mqttPayload);
					printf("MQTT Published: %s\n\r", mqttPayload);

				}
		  }

		  if(xQueueReceive(mqttQueue, &receivedState, pdMS_TO_TICKS(500)) == pdTRUE)
			{
			   sprintf(mqttPayload, "{\"device1\":%d}", receivedState);
			   MQTT_Pub("STM32/publish", mqttPayload);
			   printf("MQTT Published: %s\n\r", mqttPayload);
			   lastKeepAliveTime = millis();

			}
			if ((millis() - lastKeepAliveTime) >= (KEEP_ALIVE_INTERVAL * 500))
			{
			   uint8_t pingreq_packet[2] = {0xC0, 0x00};
			   HAL_UART_Transmit(UART_SIM800, pingreq_packet, 2, 100);
			   printf("Sending MQTT PINGREQ\n\r");
			   lastKeepAliveTime = millis();
			}
	   }
    vTaskDelay(100);
   }
}

void RTC_Task(void *pvParameters)                          // RTC Task Handler
{
	while(1)
	{
		get_time_date(timeData, dateData);
		printf("%s\n\r", timeData);
		printf("%s\n\r", dateData);
		vTaskDelay(1000);
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)                // External interrupt call back
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (GPIO_Pin == USER_BT_Pin)
    {
        xSemaphoreGiveFromISR(xButtonSemaphore, xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


#ifdef __GNUC__                                                //printf
#define UART_printf   int __io_putchar(int ch)
UART_printf
{
	HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
	return ch;
}
#endif

uint32_t millis() {
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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_SPI1_Init();
  MX_UART4_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x2345)
  {
	set_time(15, 20, 30);
	set_date(25, 02, 22, 7);
  }
  set_time(15, 20, 30);
  set_date(25, 02, 22, 7);
  HAL_UART_Receive_IT(&huart4, uartRxBuffer, sizeof(uartRxBuffer));
  HAL_UART_Receive_IT(UART_SIM800, &rx_data, 1);
  printf("system Init\n\r");
  GSM_init();
  FreeRTOS_Init();
//  uint8_t sub = 0;
//  uint8_t pub_uint8 = 1;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void set_time (uint8_t hr, uint8_t min, uint8_t sec)
{
	RTC_TimeTypeDef sTime = {0};

	sTime.Hours = hr;
	sTime.Minutes = min;
	sTime.Seconds = sec;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
}

void set_date (uint8_t year, uint8_t month, uint8_t date, uint8_t day)  // monday = 1
{

	RTC_DateTypeDef sDate = {0};
	sDate.WeekDay = day;
	sDate.Month = month;
	sDate.Date = date;
	sDate.Year = year;
	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}

	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x2345);  // backup register
}


void set_alarm (uint8_t hr, uint8_t min, uint8_t sec, uint8_t date)
{
	RTC_AlarmTypeDef sAlarm = {0};
	sAlarm.AlarmTime.Hours = hr;
	sAlarm.AlarmTime.Minutes = min;
	sAlarm.AlarmTime.Seconds = sec;
	sAlarm.AlarmTime.SubSeconds = 0;
	sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	sAlarm.AlarmDateWeekDay = date;
	sAlarm.Alarm = RTC_ALARM_A;
	if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
}

void get_time_date(char *time, char *date)
{
  RTC_DateTypeDef gDate;
  RTC_TimeTypeDef gTime;
  HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);
  sprintf((char*)time,"%02d:%02d:%02d",gTime.Hours, gTime.Minutes, gTime.Seconds);
  sprintf((char*)date,"%02d-%02d-%2d",gDate.Date, gDate.Month, 2000 + gDate.Year);
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 1);  // turn the LED ON
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
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
