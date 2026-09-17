#include "main.h"
#include "stm32f103xb.h"
#include <string.h>

UART_HandleTypeDef huart3;

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);

int main(void)
{
  uint8_t received_byte;
  char command_buffer[8] = {0};
  uint8_t command_length = 0U;
  const uint8_t startup_message[] =
      "Bluetooth HC-05 ready. Send on or off + Enter\r\n";

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART3_UART_Init();

  HAL_GPIO_WritePin(green_GPIO_Port, green_Pin, GPIO_PIN_RESET);
  HAL_UART_Transmit(&huart3, (uint8_t *)startup_message,
                    sizeof(startup_message) - 1U, HAL_MAX_DELAY);

  while (1)
  {
    if (HAL_UART_Receive(&huart3, &received_byte, 1U, HAL_MAX_DELAY) == HAL_OK)
    {
      HAL_UART_Transmit(&huart3, &received_byte, 1U, HAL_MAX_DELAY);

      if (received_byte == '\r' || received_byte == '\n')
      {
        command_buffer[command_length] = '\0';

        if (strcmp(command_buffer, "on") == 0)
        {
          HAL_GPIO_WritePin(green_GPIO_Port, green_Pin, GPIO_PIN_SET);
          HAL_UART_Transmit(&huart3, (uint8_t *)"\r\nLED ON\r\n", 10U,
                            HAL_MAX_DELAY);
        }
        else if (strcmp(command_buffer, "off") == 0)
        {
          HAL_GPIO_WritePin(green_GPIO_Port, green_Pin, GPIO_PIN_RESET);
          HAL_UART_Transmit(&huart3, (uint8_t *)"\r\nLED OFF\r\n", 11U,
                            HAL_MAX_DELAY);
        }
        else if (command_length > 0U)
        {
          HAL_UART_Transmit(&huart3, (uint8_t *)"\r\nUse on or off\r\n", 17U,
                            HAL_MAX_DELAY);
        }

        command_length = 0U;
        memset(command_buffer, 0, sizeof(command_buffer));
      }
      else if (command_length < sizeof(command_buffer) - 1U)
      {
        command_buffer[command_length++] = (char)received_byte;
      }
      else
      {
        command_length = 0U;
        memset(command_buffer, 0, sizeof(command_buffer));
      }
    }
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
  GPIO_InitTypeDef gpio_config = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  HAL_GPIO_WritePin(green_GPIO_Port, green_Pin, GPIO_PIN_RESET);

  gpio_config.Pin = green_Pin;
  gpio_config.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_config.Pull = GPIO_NOPULL;
  gpio_config.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(green_GPIO_Port, &gpio_config);
}

static void MX_USART3_UART_Init(void)
{
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
}

void HAL_UART_MspInit(UART_HandleTypeDef *uart_handle)
{
  GPIO_InitTypeDef gpio_config = {0};

  if (uart_handle->Instance == USART3)
  {
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    gpio_config.Pin = GPIO_PIN_10;
    gpio_config.Mode = GPIO_MODE_AF_PP;
    gpio_config.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio_config);

    gpio_config.Pin = GPIO_PIN_11;
    gpio_config.Mode = GPIO_MODE_INPUT;
    gpio_config.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &gpio_config);
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
