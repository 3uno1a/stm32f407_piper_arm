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
#include "can.h"
#include "i2c.h"
#include "i2s.h"
#include "spi.h"
#include "usb_host.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f4xx_hal.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

CAN_TxHeaderTypeDef TxHeader1, TxHeader2;
CAN_RxHeaderTypeDef RxHeader1, RxHeader2;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t TxData1[8] = {1,2,3,4,5,6,7,8};
uint8_t TxData2[8] = {10, 20, 30, 40, 50, 60, 70, 80};

uint8_t RxData1[8];
uint8_t RxData2[8];

uint32_t TxMailbox1, TxMailbox2;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
void Init_TxHeaders(void);
void Init_CAN_Filter(CAN_HandleTypeDef* hcan);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_I2C1_Init();
  MX_I2S3_Init();
  MX_SPI1_Init();
  MX_USB_HOST_Init();
  MX_CAN1_Init();
  MX_CAN2_Init();
  /* USER CODE BEGIN 2 */
  Init_TxHeaders();
  Init_CAN_Filter(&hcan1);
  Init_CAN_Filter(&hcan2);


  HAL_CAN_Start(&hcan1);
  HAL_CAN_Start(&hcan2);

  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
  HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);

  uint32_t tick1 = HAL_GetTick();
  uint32_t tick2 = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    MX_USB_HOST_Process();

    /* USER CODE BEGIN 3 */
    uint32_t now = HAL_GetTick();

    if (now - tick1 >= 1000)
    {
      HAL_CAN_AddTxMessage(&hcan1, &TxHeader1, TxData1, &TxMailbox1);
      tick1 = now;
    }

    if (now - tick2 >= 2000)
    {
      HAL_CAN_AddTxMessage(&hcan2, &TxHeader2, TxData2, &TxMailbox2);
      tick2 = now;
    }
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  if (hcan -> Instance == CAN2)
  {
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader2, RxData2);
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);     // green
  }
  else if (hcan -> Instance == CAN1)
  {
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader1, RxData1);
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);    // red
  }
}

void Init_TxHeaders(void)
{
  TxHeader1.DLC = 8;
  TxHeader1.StdId = 0x100;     // CAN1 -> CAN2 msg ID
  TxHeader1.ExtId = 0;
  TxHeader1.IDE = CAN_ID_STD;
  TxHeader1.RTR = CAN_RTR_DATA;
  TxHeader1.TransmitGlobalTime = DISABLE;

  TxHeader2.DLC = 8;
  TxHeader2.StdId = 0x200;     // CAN2 -> CAN1 msg ID
  TxHeader2.ExtId = 0;
  TxHeader2.IDE = CAN_ID_STD;
  TxHeader2.RTR = CAN_RTR_DATA;
  TxHeader2.TransmitGlobalTime = DISABLE;
}

void Init_CAN_Filter(CAN_HandleTypeDef* hcan)
{
  CAN_FilterTypeDef canFilter = {
      .FilterActivation = ENABLE,
      .FilterBank = 0,
      .FilterFIFOAssignment = CAN_FILTER_FIFO0,
      .FilterIdHigh = 0x0000,
      .FilterIdLow = 0x0000,
      .FilterMaskIdHigh = 0x0000,
      .FilterMaskIdLow = 0x0000,
      .FilterMode = CAN_FILTERMODE_IDMASK,
      .FilterScale = CAN_FILTERSCALE_32BIT
  };
  HAL_CAN_ConfigFilter(hcan, &canFilter);
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
