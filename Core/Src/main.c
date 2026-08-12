/* USER CODE BEGIN Header */
// sudo minicom -D /dev/ttyACM0 -b 115200

/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "arena.h"
#include "csv_render.h"
#include "fatfs.h"
#include "usb_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f4xx_hal_uart.h"
#include "touchscreen.h"
#include "csv.h"
#include "csv_render.h"
#include "stm32412g_discovery.h"
#include "stm32412g_discovery_lcd.h"
#include "stm32f412zx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_cortex.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc_ex.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32412g_discovery_ts.h"
#include "usbh_hid.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "fs_browser.h"
#include "fs_render.h"
#include "menu_render.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    STATE_MENU,
    STATE_TABLE,
    STATE_FS_BROWSER
} AppState;

typedef struct {
  char buff[MAX_LEN_LINE];
  int cnt;
  int edit_row;
  int edit_col;
  uint8_t need_rendering;
} KBParams;

typedef struct {
  int selected_entry;
  uint16_t count_fentries;
} FSParams;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SD_HandleTypeDef hsd;

TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;

SRAM_HandleTypeDef hsram1;

/* USER CODE BEGIN PV */
volatile JOYState_TypeDef StableJoyState = JOY_NONE;
volatile uint8_t joy_flag = 0;
volatile uint8_t ts_flag = 0;
static TS_StateTypeDef TS_State = {0};
uint8_t uart_rx_byte;
volatile uint8_t newChar_flag = 0;

extern FS_Entry entries_buff[MAX_ENTRIES];
extern char current_path[MAX_PATH_LEN];
extern uint8_t is_tracking;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_FSMC_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM6_Init(void);
static void MX_SDIO_SD_Init(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
void process_menu_state(AppState* cur_state, Table** table, MenuParams* menu_params, FSParams* fs_params, RenderParams* render_params, KBParams* kb_params);
void process_table_state(AppState* cur_state, Table** table, MenuParams* menu_params, RenderParams* render_params, KBParams* kb_params);
void process_fs_browser_state(AppState* cur_state, Table** table, FSParams* fs_params, RenderParams* render_params, KBParams* kb_params);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
const char* csv_data[] = {
                ",A,B,C,VeryLongHeaderNameTooLongFloat,NextColumn\n"
                      "1,0,0,1,12,\n"
                      "2,2,=A1+C30,0,3,1\n"
                      "30,0,=6/B1,5,6,-2\n"
                      "32,1,15,6,,0\n"
                      "35,-45,22,0,=B2,0\n"
                      "39,7,,,,\n"
                      "42,3,2005,6,8,-1\n"
                      "52,11,25,6,,9\n"
                      "72,1,1,1,,0\n"
                      "30,1,1,6,,0\n"
                      "3210900,0,=B2*0,5,1,-5\n", 

                  ",A,B,C,Very,Next,Cell\n"
                      "1,0,0,1,12,,1\n"
                      "2,2,=A1+C30,0,3,1,2\n"
                      "30,0,=6/B1,5,6,-2,23\n"
                      "32,1,15,6,,0,4444\n"
                      "35,-45,22,0,=B2,0,678\n"
                      "39,7,,,,,0\n"
                      "42,3,2005,6,8,-1,6\n"
                      "52,11,25,6,,9,67\n"
                      "72,1,1,1,,0,42\n"
                      "30,1,1,6,,0,13\n"
                      "3210900,0,=B2*0,5,1,-5,-120\n", 
                    
                  ",A\n"
                      "1,3\n"
                      "2,2,\n"
                      "30,0,\n"
                      "32,1\n"
                      "35,-45\n"
                      "39,7\n"
                      "42,3\n"
                      "52,11\n"
                      "72,1\n"
                      "30,1\n"
                      "3210900,0\n", 
                    };

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
  MX_FSMC_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_TIM6_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  MX_USB_HOST_Init();
  /* USER CODE BEGIN 2 */
  init_lcd();

  uint8_t status = 0;
  status = BSP_JOY_Init(JOY_MODE_GPIO);
  if (status != HAL_OK) {
      display_error("Failed to initialize joystick");
  }

  uint32_t ts_status = TS_OK;
  ts_status = BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
  ts_status |= BSP_TS_ITConfig();

  if (ts_status != TS_OK) {
      display_error("Failed to initialize touchscreen");
  }

  AppState cur_state = STATE_MENU;
  Table* table = NULL;

  MenuParams menu_params = {
    .selected_table = 0, 
    .total_tables = 3
  };
  RenderParams render_params = {0};
  FSParams fs_params = {0};
  KBParams kb_params = {0};

  display_main_menu(&menu_params);

  HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1);
  // char buff[MAX_LEN_LINE];
  // int cnt = 0;
  // int edit_row = -2;
  // int edit_col = -2;
  // uint8_t need_rendering = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
   switch (cur_state) {
      case STATE_MENU:
        process_menu_state(&cur_state, &table, &menu_params, &fs_params, &render_params, &kb_params);
        break;

      case STATE_TABLE:
        process_table_state(&cur_state, &table, &menu_params, &render_params, &kb_params);
        break;

      case STATE_FS_BROWSER:
        process_fs_browser_state(&cur_state, &table, &fs_params, &render_params, &kb_params);
        break;

      default:
        break;
   }

    /* USER CODE END WHILE */
    //MX_USB_HOST_Process();
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
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

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SDIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDIO_SD_Init(void)
{

  /* USER CODE BEGIN SDIO_Init 0 */

  /* USER CODE END SDIO_Init 0 */

  /* USER CODE BEGIN SDIO_Init 1 */

  /* USER CODE END SDIO_Init 1 */
  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv = 118;
  /* USER CODE BEGIN SDIO_Init 2 */

  /* USER CODE END SDIO_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 71;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 999;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */
  __HAL_RCC_TIM6_CLK_ENABLE(); // 
  HAL_NVIC_SetPriority(TIM6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM6_IRQn);

  HAL_TIM_Base_Init(&htim6);
  HAL_TIM_Base_Start_IT(&htim6);
  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, LED3_Pin|LED4_Pin|LED1_Pin|LED2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, LCD_BLCTRL_Pin|EXT_RESET_Pin|CTP_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED3_Pin LED4_Pin LED1_Pin LED2_Pin */
  GPIO_InitStruct.Pin = LED3_Pin|LED4_Pin|LED1_Pin|LED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_BLCTRL_Pin EXT_RESET_Pin CTP_RST_Pin */
  GPIO_InitStruct.Pin = LCD_BLCTRL_Pin|EXT_RESET_Pin|CTP_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : JOY_SEL_Pin */
  GPIO_InitStruct.Pin = JOY_SEL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(JOY_SEL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : JOY_RIGHT_Pin JOY_LEFT_Pin */
  GPIO_InitStruct.Pin = JOY_RIGHT_Pin|JOY_LEFT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : JOY_UP_Pin JOY_DOWN_Pin LCD_TE_Pin */
  GPIO_InitStruct.Pin = JOY_UP_Pin|JOY_DOWN_Pin|LCD_TE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_RESET_Pin */
  GPIO_InitStruct.Pin = LCD_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CTP_INT_Pin */
  GPIO_InitStruct.Pin = CTP_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(CTP_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PG8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : uSD_Detect_Pin */
  GPIO_InitStruct.Pin = uSD_Detect_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(uSD_Detect_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : I2C2_SDA_Pin */
  GPIO_InitStruct.Pin = I2C2_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_I2C2;
  HAL_GPIO_Init(I2C2_SDA_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* FSMC initialization function */
static void MX_FSMC_Init(void)
{

  /* USER CODE BEGIN FSMC_Init 0 */

  /* USER CODE END FSMC_Init 0 */

  FSMC_NORSRAM_TimingTypeDef Timing = {0};

  /* USER CODE BEGIN FSMC_Init 1 */

  /* USER CODE END FSMC_Init 1 */

  /** Perform the SRAM1 memory initialization sequence
  */
  hsram1.Instance = FSMC_NORSRAM_DEVICE;
  hsram1.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FSMC_NORSRAM_BANK1;
  hsram1.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  hsram1.Init.ContinuousClock = FSMC_CONTINUOUS_CLOCK_SYNC_ONLY;
  hsram1.Init.WriteFifo = FSMC_WRITE_FIFO_ENABLE;
  hsram1.Init.PageSize = FSMC_PAGE_SIZE_NONE;
  /* Timing */
  Timing.AddressSetupTime = 15;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 255;
  Timing.BusTurnAroundDuration = 15;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;
  /* ExtTiming */

  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FSMC_Init 2 */

  /* USER CODE END FSMC_Init 2 */
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  static int count = 0;
  static JOYState_TypeDef newStableState = 0;

  JOYState_TypeDef current_state = BSP_JOY_GetState();

  if (htim->Instance == TIM6) {
    if (newStableState == current_state) {
      count++;
    }
    else {
      newStableState = current_state;
      count = 0;
    };

    uint8_t needed_ticks = (newStableState == JOY_NONE) ? 40 : 10; // 40 ms for release 10 ms for new state stable
    if (count == needed_ticks) {
      if (StableJoyState != newStableState) {
        if (newStableState != JOY_NONE && StableJoyState == JOY_NONE) {
          joy_flag = 1;
        }
        StableJoyState = newStableState;
      }
    }
  }
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == CTP_INT_Pin) {
    ts_flag = 1;
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART2) {
      newChar_flag = 1;
      HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1);
  }
}

void process_menu_state(AppState* cur_state, Table** table, MenuParams* menu_params, FSParams* fs_params, RenderParams* render_params, KBParams* kb_params){
  if (*table != NULL) {
    free_table(*table);
    *table = NULL;
  }
  if (!joy_flag) {
    return;
  }
  
  joy_flag = 0;
  switch (StableJoyState) {
    case JOY_UP:{
      if (menu_params->selected_table > 0) {
        menu_params->selected_table--;
        display_main_menu(menu_params);
      }
      break;
    }
            
    case JOY_DOWN:{
      if (menu_params->selected_table < menu_params->total_tables) {
        menu_params->selected_table++;
        display_main_menu(menu_params);
      }
      break;
    }

    case JOY_SEL:{
      if (menu_params->selected_table == menu_params->total_tables) {
        if (!fs_browser_mount()) {
          display_error("Failed to mount SD card");
          Error_Handler();
        }
        fs_params->count_fentries = display_fs_browser(fs_params->selected_entry);

        *cur_state = STATE_FS_BROWSER;
        break;
      }

      *table = read_csv_from_strmem(csv_data[menu_params->selected_table]);
      if (*table == NULL) {
        display_error("Failed to parse CSV data");
        break;
      }

      evaluate_all(*table);
      *render_params = (RenderParams){0};

      render_table_to_lcd(*table, render_params->start_row, render_params->start_col);
      highlight_cell(*table, 
          render_params->new_row, render_params->new_col, 
          render_params->start_row, render_params->start_col);

      *cur_state = STATE_TABLE;
      StableJoyState = JOY_NONE;
      break;
    }

    default:
      break;
  }
}

void process_table_state(AppState* cur_state, Table** table, MenuParams* menu_params, RenderParams* render_params, KBParams* kb_params) {
  if (ts_flag || is_tracking) {
    ts_flag = 0;

    BSP_TS_GetState(&TS_State);
    calibrate_coords(&TS_State.touchX[0], &TS_State.touchY[0]);

    uint16_t click_x = 0, click_y = 0;
    uint8_t gest_id = getGestureID(&TS_State, &click_x, &click_y);

    int old_s_row = render_params->start_row;
    int old_s_col = render_params->start_col;

    if (gest_id != GEST_ID_NO_GESTURE) {          
      switch (gest_id) {
        case GEST_ID_MOVE_LEFT:
          if (can_scroll_right(*table, render_params->start_row, render_params->start_col)) render_params->start_col++;
          break;
        case GEST_ID_MOVE_RIGHT:
          if (render_params->start_col > 0) render_params->start_col--;
          break;
        case GEST_ID_MOVE_UP:
          if (can_scroll_down(*table, render_params->start_row)) render_params->start_row++;
          break;
        case GEST_ID_MOVE_DOWN:
          if (render_params->start_row > 0) render_params->start_row--;
          break;
        case GEST_ID_CLICK: {
          int clicked_row = get_clicked_row(render_params->start_row, click_y);
          int clicked_col = get_clicked_col(*table, render_params->start_col, render_params->start_row, click_x);

          if (clicked_row >= -1 && clicked_col >= -1 && (clicked_row != -1 || clicked_col != -1)) {
            unhighlight_cell(*table, render_params->new_row, render_params->new_col, render_params->start_row, render_params->start_col);
            
            render_params->new_row = clicked_row;
            render_params->new_col = clicked_col;
            close_edit_mode(*table, &kb_params->edit_row, &kb_params->edit_col, render_params->start_row, render_params->start_col, render_params->viewport_changed, &kb_params->need_rendering);
            kb_params->cnt = 0;
            kb_params->buff[0] = '\0';

            highlight_cell(*table, render_params->new_row, render_params->new_col, render_params->start_row, render_params->start_col);
          }
          break;
        }
      }
    }

    if (render_params->start_col != old_s_col || render_params->start_row != old_s_row) {
      render_table_to_lcd(*table, render_params->start_row, render_params->start_col);
      highlight_cell(*table, render_params->new_row, render_params->new_col, render_params->start_row, render_params->start_col); 
    }
  }

  if (joy_flag) {
    joy_flag = 0;
    render_params->prev_row = render_params->new_row;
    render_params->prev_col = render_params->new_col;

    switch (StableJoyState) {
      case JOY_UP:
        if (render_params->new_row - render_params->start_row >= -1) {
          if ((render_params->new_row == 0 && render_params->new_col == -1) || render_params->new_row == -1) break;
          render_params->new_row--;
        }
        break;
      case JOY_DOWN:
        if (render_params->new_row < (*table)->row_count - 1) {
            render_params->new_row++;
        }
        break;     
      case JOY_LEFT:
        if (render_params->new_col - render_params->start_col >= -1) {
          if ((render_params->new_col == 0 && render_params->new_row == -1) || render_params->new_col == -1) break;
            render_params->new_col--;
        }
        break;
      case JOY_RIGHT:
        if (render_params->new_col < (*table)->col_count - 1) {
            render_params->new_col++;
        }
        break;

      case JOY_SEL:
        if (menu_params->selected_table == menu_params->total_tables) {
          save_table(*table, current_path);
        }
        free_table(*table);
        *table = NULL;
        display_main_menu(menu_params);
        StableJoyState = JOY_NONE;
        *cur_state = STATE_MENU;
        return;

      default:
        break;
    }

    if (StableJoyState != JOY_NONE) {
        close_edit_mode(*table, &kb_params->edit_row, &kb_params->edit_col, render_params->start_row, render_params->start_col, render_params->viewport_changed, &kb_params->need_rendering);
        kb_params->cnt = 0;
        kb_params->buff[0] = '\0';
        update_viewport(render_params->new_row, render_params->new_col, &render_params->start_row, &render_params->start_col, *table, &render_params->viewport_changed);

        if (render_params->viewport_changed) {
            render_table_to_lcd(*table, render_params->start_row, render_params->start_col);
        } else {
            unhighlight_cell(*table, render_params->prev_row, render_params->prev_col, render_params->start_row, render_params->start_col);
        }

        if (!highlight_cell(*table, render_params->new_row, render_params->new_col, render_params->start_row, render_params->start_col)) {
          render_params->new_row = render_params->prev_row;
          render_params->new_col = render_params->prev_col;
          highlight_cell(*table, render_params->new_row, render_params->new_col, render_params->start_row, render_params->start_col);
        }
    }  
  }

  if (newChar_flag && render_params->new_row != -1 && render_params->new_col != -1) {
    int dummyX = 0, dummyY = 0;
    find_cell_pos(*table, render_params->new_row, render_params->new_col, &dummyX, &dummyY, render_params->start_row, render_params->start_col);
    dummyY += LCD_DEFAULT_FONT.Height / 2;
    Cell* cell = &(*table)->grid[render_params->new_row * (*table)->col_count + render_params->new_col];

    if (kb_params->cnt == 0 && kb_params->buff[0] == '\0') {
      kb_params->edit_col = render_params->new_col;
      kb_params->edit_row = render_params->new_row;
      if (cell->raw_data != NULL) {
        kb_params->cnt = strlen(cell->raw_data);
        strncpy(kb_params->buff, cell->raw_data, kb_params->cnt);
      } else {
        kb_params->cnt = 0;
      }
      
      kb_params->buff[kb_params->cnt] = '\0';
      clear_cell(*table, render_params->new_row, render_params->new_col, render_params->start_row, render_params->start_col, (uint16_t)0x74FA);
    }

    if (uart_rx_byte == '\n' || uart_rx_byte == '\r') {
      kb_params->buff[kb_params->cnt] = '\0';
      if (render_params->new_row != -1 && render_params->new_col != -1) {
        int old_max_col_w = get_max_col_len(*table, render_params->start_row, render_params->new_col, render_params->start_col);
        cell->raw_data = arena_strdup(&table_arena, kb_params->buff);
        cell->state = RAW;
        if (evaluate_all(*table)) {
          kb_params->need_rendering = 1;
        }
        if (old_max_col_w != get_max_col_len(*table, render_params->start_row, render_params->new_col, render_params->start_col) || kb_params->need_rendering) {
          update_viewport(render_params->new_row, render_params->new_col, &render_params->start_row, &render_params->start_col, *table, &render_params->viewport_changed);
          render_table_to_lcd(*table, render_params->start_row, render_params->start_col);
          kb_params->need_rendering = 0;
        }
        highlight_cell(*table, render_params->new_row, render_params->new_col, render_params->start_row, render_params->start_col);
      }
      kb_params->cnt = 0;
      kb_params->buff[0] = '\0';
      kb_params->edit_row = -2;
      kb_params->edit_col = -2;

    } else {
      if (kb_params->cnt < MAX_LEN_FIELD) {
        if (uart_rx_byte == '\b' && kb_params->cnt > 0) {
          kb_params->buff[kb_params->cnt] = '\0';
          kb_params->buff[--kb_params->cnt] = '\b';
          int cur_col = get_clicked_col(*table, render_params->start_col, render_params->start_row, dummyX + (kb_params->cnt + 1) * LCD_DEFAULT_FONT.Width);
          uint16_t cur_color = get_cell_color(kb_params->edit_row, cur_col);
          BSP_LCD_SetTextColor(cur_color);
          BSP_LCD_SetBackColor(cur_color);
          BSP_LCD_FillRect(dummyX + (kb_params->cnt + 1) * LCD_DEFAULT_FONT.Width, dummyY - LCD_DEFAULT_FONT.Height / 2, LCD_DEFAULT_FONT.Width, 24);
          BSP_LCD_SetBackColor((uint16_t)0x74FA);
          int dummyx2 = 0;
          find_cell_pos(*table, kb_params->edit_row, cur_col, &dummyx2, &(int){0}, render_params->start_row, render_params->start_col);
          if (dummyx2 > dummyX + (kb_params->cnt) * LCD_DEFAULT_FONT.Width) {
            draw_cell(*table, kb_params->edit_row, cur_col, &(int){0}, &(int){0}, render_params->start_row, render_params->start_col, cur_color, 0);
            BSP_LCD_SetBackColor((uint16_t)0x74FA);
          }
        } else if (uart_rx_byte != '\b') {
          kb_params->buff[kb_params->cnt++] = uart_rx_byte;
          kb_params->buff[kb_params->cnt] = '\0';
        }

        int cell_w = get_max_col_len(*table, render_params->start_row, render_params->new_col, render_params->start_col);
        if (cell_w < (kb_params->cnt + 1) * LCD_DEFAULT_FONT.Width) {
          cell_w = (kb_params->cnt + 1) * LCD_DEFAULT_FONT.Width;
          kb_params->need_rendering = 1;
        }
        if (dummyX + (kb_params->cnt + 1) * LCD_DEFAULT_FONT.Width > 240) {
          kb_params->buff[--kb_params->cnt] = '\0';
          cell_w -= LCD_DEFAULT_FONT.Width;
        }

        BSP_LCD_SetTextColor((uint16_t)0x74FA);
        BSP_LCD_FillRect(dummyX, dummyY - LCD_DEFAULT_FONT.Height / 2, cell_w, 24);
        BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
        BSP_LCD_DisplayStringAt(dummyX, dummyY, (uint8_t*)kb_params->buff, 0);
      }

      BSP_LCD_DisplayChar(dummyX + (kb_params->cnt) * LCD_DEFAULT_FONT.Width, dummyY, '_');
    }

    newChar_flag = 0;
  }
}

void process_fs_browser_state(AppState* cur_state, Table** table, FSParams* fs_params, RenderParams* render_params, KBParams* kb_params) {
  if (joy_flag) {
    joy_flag = 0;
    switch (StableJoyState) {
      case JOY_UP:
        if (fs_params->selected_entry > 0) {
          fs_params->selected_entry--;
          fs_params->count_fentries = display_fs_browser(fs_params->selected_entry);
        }
        break;
        
      case JOY_DOWN:
        if (fs_params->selected_entry < fs_params->count_fentries - 1) {
          fs_params->selected_entry++;
          fs_params->count_fentries = display_fs_browser(fs_params->selected_entry);
        }
        break;

      case JOY_SEL:
        if (!entries_buff[fs_params->selected_entry].is_dir) {
          fs_path_append(current_path, entries_buff[fs_params->selected_entry].name);
          *table = read_csv_from_file(entries_buff[fs_params->selected_entry].name);
          if (*table == NULL) {
            display_error("Failed to parse CSV data");
            break;
          }
          evaluate_all(*table);

          *render_params = (RenderParams){0};
          *kb_params = (KBParams){ .edit_row = -2, .edit_col = -2 };

          render_table_to_lcd(*table, render_params->start_row, render_params->start_col);
          highlight_cell(*table, render_params->new_row, render_params->new_col, render_params->start_row, render_params->start_col);

          *cur_state = STATE_TABLE;
          StableJoyState = JOY_NONE;
        } else {
          if (strcmp(entries_buff[fs_params->selected_entry].name, "..") == 0) {
            fs_path_remove_last(current_path);
          } else {
            fs_path_append(current_path, entries_buff[fs_params->selected_entry].name);
          }
          fs_params->selected_entry = 0;
          fs_params->count_fentries = display_fs_browser(fs_params->selected_entry);
        }
        break;

      default:
        break;
    }
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
#ifdef USE_FULL_ASSERT
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
