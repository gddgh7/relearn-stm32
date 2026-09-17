#include "main.h"
#include <stdio.h>
#include <string.h>

RTC_HandleTypeDef hrtc;
UART_HandleTypeDef huart2;

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_USART2_UART_Init(void);
static void Print_Current_Time(RTC_DateTypeDef date, RTC_TimeTypeDef time);

int main(void)
{
  RTC_DateTypeDef date = {0};
  RTC_TimeTypeDef time = {0};
  uint32_t last_second = UINT32_MAX;
  const uint8_t startup_message[] =
      "RTC ready. Time will be printed once per second.\r\n";

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_USART2_UART_Init();

  HAL_UART_Transmit(&huart2, (uint8_t *)startup_message,
                    sizeof(startup_message) - 1U, HAL_MAX_DELAY);

  while (1)
  {
    if (HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN) != HAL_OK)
    {
      Error_Handler();
    }
    if (HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN) != HAL_OK)
    {
      Error_Handler();
    }

    if (time.Seconds != last_second)
    {
      last_second = time.Seconds;
      Print_Current_Time(date, time);
    }

    HAL_Delay(100U);
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
}

static void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
}

static void MX_RTC_Init(void)
{
  RTC_TimeTypeDef initial_time = {0};
  RTC_DateTypeDef initial_date = {0};
  RCC_OscInitTypeDef osc_init = {0};
  RCC_PeriphCLKInitTypeDef periph_clk = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_BKP_CLK_ENABLE();
  SET_BIT(PWR->CR, PWR_CR_DBP);

  osc_init.OscillatorType = RCC_OSCILLATORTYPE_LSI;
  osc_init.LSIState = RCC_LSI_ON;
  if (HAL_RCC_OscConfig(&osc_init) != HAL_OK)
  {
    Error_Handler();
  }

  periph_clk.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  periph_clk.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&periph_clk) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_RCC_RTC_ENABLE();

  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = 127U;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_RTC_GetDate(&hrtc, &initial_date, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_RTC_GetTime(&hrtc, &initial_time, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }

  if (initial_date.Year == 0U && initial_time.Hours == 0U &&
      initial_time.Minutes == 0U && initial_time.Seconds == 0U)
  {
    initial_date.Year = 26U;
    initial_date.Month = RTC_MONTH_SEPTEMBER;
    initial_date.Date = 14U;
    initial_date.WeekDay = RTC_WEEKDAY_MONDAY;
    if (HAL_RTC_SetDate(&hrtc, &initial_date, RTC_FORMAT_BIN) != HAL_OK)
    {
      Error_Handler();
    }

    initial_time.Hours = 12U;
    initial_time.Minutes = 0U;
    initial_time.Seconds = 0U;
    if (HAL_RTC_SetTime(&hrtc, &initial_time, RTC_FORMAT_BIN) != HAL_OK)
    {
      Error_Handler();
    }
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

static void Print_Current_Time(RTC_DateTypeDef date, RTC_TimeTypeDef time)
{
  char line[64];
  int length = snprintf(line, sizeof(line),
                        "RTC %04u-%02u-%02u %02u:%02u:%02u\r\n",
                        2000U + (unsigned int)date.Year, (unsigned int)date.Month,
                        (unsigned int)date.Date, (unsigned int)time.Hours,
                        (unsigned int)time.Minutes, (unsigned int)time.Seconds);

  if (length > 0)
  {
    HAL_UART_Transmit(&huart2, (uint8_t *)line, (uint16_t)length, HAL_MAX_DELAY);
  }
}

void HAL_RTC_MspInit(RTC_HandleTypeDef *rtc_handle)
{
  if (rtc_handle->Instance == RTC)
  {
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_BKP_CLK_ENABLE();
    SET_BIT(PWR->CR, PWR_CR_DBP);
    __HAL_RCC_RTC_ENABLE();
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

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
