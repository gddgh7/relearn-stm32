#include "main.h"
#include <stdio.h>
#include <string.h>

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;

#define AHT20_I2C_ADDR 0x38U
#define AHT20_I2C_WRITE_ADDR ((AHT20_I2C_ADDR << 1U) | 0x00U)
#define AHT20_I2C_READ_ADDR ((AHT20_I2C_ADDR << 1U) | 0x01U)
#define AHT20_CMD_INIT 0xBEU
#define AHT20_CMD_MEASURE 0xACU

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static uint8_t AHT20_ReadStatus(void);
static uint8_t AHT20_Init(void);
static uint8_t AHT20_Measure(void);
static uint8_t AHT20_ReadData(float *humidity, float *temperature);

int main(void)
{
  const uint8_t startup_message[] =
      "AHT20 ready. Reading temperature and humidity via I2C...\r\n";
  char message[96];
  float humidity = 0.0f;
  float temperature = 0.0f;

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();

  HAL_UART_Transmit(&huart2, (uint8_t *)startup_message,
                    sizeof(startup_message) - 1U, HAL_MAX_DELAY);

  if (AHT20_Init() != 0U)
  {
    HAL_UART_Transmit(&huart2, (uint8_t *)"Init failed\r\n", 13U, HAL_MAX_DELAY);
    Error_Handler();
  }

  while (1)
  {
    if (AHT20_Measure() == 0U)
    {
      if (AHT20_ReadData(&humidity, &temperature) == 0U)
      {
        int humidity_whole = (int)humidity;
        int humidity_frac = (int)((humidity - (float)humidity_whole) * 100.0f + 0.5f);
        int temperature_whole = (int)temperature;
        int temperature_frac =
            (int)((temperature - (float)temperature_whole) * 100.0f + 0.5f);

        if (humidity_frac >= 100)
        {
          humidity_frac = 0;
          humidity_whole += 1;
        }
        if (temperature_frac >= 100)
        {
          temperature_frac = 0;
          temperature_whole += 1;
        }

        int length = snprintf(message, sizeof(message),
                              "RH: %d.%02d %%  TEMP: %d.%02d C\r\n",
                              humidity_whole, humidity_frac, temperature_whole,
                              temperature_frac);
        HAL_UART_Transmit(&huart2, (uint8_t *)message, (uint16_t)length,
                          HAL_MAX_DELAY);
      }
      else
      {
        HAL_UART_Transmit(&huart2, (uint8_t *)"Read failed\r\n",
                          14U, HAL_MAX_DELAY);
      }
    }
    else
    {
      HAL_UART_Transmit(&huart2, (uint8_t *)"AHT20 busy\r\n",
                        13U, HAL_MAX_DELAY);
    }

    HAL_Delay(2000U);
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

static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000U;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0U;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0U;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
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

static uint8_t AHT20_ReadStatus(void)
{
  uint8_t status = 0U;
  if (HAL_I2C_Master_Receive(&hi2c1, AHT20_I2C_READ_ADDR,
                             &status, 1U, HAL_MAX_DELAY) != HAL_OK)
  {
    return 0xFFU;
  }
  return status;
}

static uint8_t AHT20_Init(void)
{
  uint8_t command[3] = {AHT20_CMD_INIT, 0x08U, 0x00U};
  if (HAL_I2C_Master_Transmit(&hi2c1, AHT20_I2C_WRITE_ADDR, command,
                              sizeof(command), HAL_MAX_DELAY) != HAL_OK)
  {
    return 1U;
  }

  HAL_Delay(40U);
  for (uint8_t i = 0U; i < 20U; ++i)
  {
    if ((AHT20_ReadStatus() & 0x80U) == 0U)
    {
      return 0U;
    }
    HAL_Delay(10U);
  }

  return 1U;
}

static uint8_t AHT20_Measure(void)
{
  uint8_t command[3] = {AHT20_CMD_MEASURE, 0x33U, 0x00U};
  if (HAL_I2C_Master_Transmit(&hi2c1, AHT20_I2C_WRITE_ADDR, command,
                              sizeof(command), HAL_MAX_DELAY) != HAL_OK)
  {
    return 1U;
  }

  HAL_Delay(80U);

  for (uint8_t i = 0U; i < 50U; ++i)
  {
    if ((AHT20_ReadStatus() & 0x80U) == 0U)
    {
      return 0U;
    }
    HAL_Delay(10U);
  }

  return 1U;
}

static uint8_t AHT20_ReadData(float *humidity, float *temperature)
{
  uint8_t data[6] = {0U};
  uint32_t humidity_raw;
  uint32_t temperature_raw;
  uint8_t status = AHT20_ReadStatus();

  if ((status & 0x80U) != 0U)
  {
    return 1U;
  }

  if (HAL_I2C_Master_Receive(&hi2c1, AHT20_I2C_READ_ADDR, data, sizeof(data),
                             HAL_MAX_DELAY) != HAL_OK)
  {
    return 1U;
  }

  if ((data[0] & 0x80U) != 0U)
  {
    return 1U;
  }

  {
    char debug_msg[64];
    int length = snprintf(debug_msg, sizeof(debug_msg),
                          "RAW: %02X %02X %02X %02X %02X %02X\r\n",
                          data[0], data[1], data[2], data[3], data[4], data[5]);
    HAL_UART_Transmit(&huart2, (uint8_t *)debug_msg, (uint16_t)length,
                      HAL_MAX_DELAY);
  }

  humidity_raw = ((uint32_t)data[1] << 12U) |
                 ((uint32_t)data[2] << 4U) |
                 ((uint32_t)data[3] >> 4U);
  temperature_raw = (((uint32_t)data[3] & 0x0FU) << 16U) |
                    ((uint32_t)data[4] << 8U) |
                    (uint32_t)data[5];

  *humidity = (float)humidity_raw * 100.0f / 1048576.0f;
  *temperature = ((float)temperature_raw * 200.0f / 1048576.0f) - 50.0f;

  return 0U;
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *i2c_handle)
{
  GPIO_InitTypeDef gpio_config = {0};

  if (i2c_handle->Instance == I2C1)
  {
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    gpio_config.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio_config.Mode = GPIO_MODE_AF_OD;
    gpio_config.Pull = GPIO_PULLUP;
    gpio_config.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio_config);
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
