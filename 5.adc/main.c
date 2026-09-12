#include "main.h"
#include <stdio.h>

ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart2;

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);
static void Error_Message(void);

int main(void)
{
  uint32_t adc_value;
  uint32_t voltage_mv;
  char message[64];

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();

  if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_UART_Transmit(&huart2, (uint8_t *)"ADC1 PA5 ready\r\n", 17U,
                    HAL_MAX_DELAY);

  while (1)
  {
    if (HAL_ADC_Start(&hadc1) != HAL_OK ||
        HAL_ADC_PollForConversion(&hadc1, 100U) != HAL_OK)
    {
      Error_Message();
    }

    adc_value = HAL_ADC_GetValue(&hadc1);
    voltage_mv = adc_value * 3300U / 4095U;
    int message_length = snprintf(message, sizeof(message),
                                   "ADC: %lu, Voltage: %lu mV\r\n",
                                   (unsigned long)adc_value,
                                   (unsigned long)voltage_mv);
    HAL_UART_Transmit(&huart2, (uint8_t *)message, (uint16_t)message_length,
                      HAL_MAX_DELAY);
    HAL_ADC_Stop(&hadc1);
    HAL_Delay(500);
  }
}

static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef osc_config = {0};
  RCC_ClkInitTypeDef clock_config = {0};

  osc_config.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  osc_config.HSEState = RCC_HSE_ON;
  osc_config.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  osc_config.HSIState = RCC_HSI_ON;
  osc_config.PLL.PLLState = RCC_PLL_ON;
  osc_config.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  osc_config.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&osc_config) != HAL_OK)
  {
    Error_Handler();
  }

  clock_config.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                           RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  clock_config.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clock_config.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clock_config.APB1CLKDivider = RCC_HCLK_DIV2;
  clock_config.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&clock_config, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  MODIFY_REG(RCC->CFGR, RCC_CFGR_ADCPRE, RCC_CFGR_ADCPRE_DIV6);
}

static void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
}

static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef channel_config = {0};

  __HAL_RCC_ADC1_CLK_ENABLE();
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1U;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  channel_config.Channel = ADC_CHANNEL_5;
  channel_config.Rank = ADC_REGULAR_RANK_1;
  channel_config.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &channel_config) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_USART2_UART_Init(void)
{
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
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *adc_handle)
{
  GPIO_InitTypeDef gpio_config = {0};

  if (adc_handle->Instance == ADC1)
  {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    gpio_config.Pin = GPIO_PIN_5;
    gpio_config.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &gpio_config);
  }
}

void HAL_UART_MspInit(UART_HandleTypeDef *uart_handle)
{
  GPIO_InitTypeDef gpio_config = {0};

  if (uart_handle->Instance == USART2)
  {
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    gpio_config.Pin = GPIO_PIN_2;
    gpio_config.Mode = GPIO_MODE_AF_PP;
    gpio_config.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio_config);

    gpio_config.Pin = GPIO_PIN_3;
    gpio_config.Mode = GPIO_MODE_INPUT;
    gpio_config.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &gpio_config);
  }
}

static void Error_Message(void)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)"ADC error\r\n", 11U, HAL_MAX_DELAY);
  Error_Handler();
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
