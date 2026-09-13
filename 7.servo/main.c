#include "main.h"
#include <stdlib.h>
#include <string.h>

TIM_HandleTypeDef htim4;
UART_HandleTypeDef huart2;

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART2_UART_Init(void);
static void Set_Servo_Angle(uint32_t angle);

int main(void)
{
  uint8_t received_byte;
  char command_buffer[4] = {0};
  uint8_t command_length = 0U;
  const uint8_t startup_message[] =
      "Servo PB8 ready. Send 0, 90 or 180 + Enter\r\n";

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM4_Init();

  if (HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  Set_Servo_Angle(90U);

  HAL_UART_Transmit(&huart2, (uint8_t *)startup_message,
                    sizeof(startup_message) - 1U, HAL_MAX_DELAY);

  while (1)
  {
    if (HAL_UART_Receive(&huart2, &received_byte, 1U, HAL_MAX_DELAY) == HAL_OK)
    {
      HAL_UART_Transmit(&huart2, &received_byte, 1U, HAL_MAX_DELAY);

      if (received_byte == '\r' || received_byte == '\n')
      {
        command_buffer[command_length] = '\0';
        uint32_t angle = (uint32_t)strtoul(command_buffer, NULL, 10);

        if (command_length > 0U && angle <= 180U)
        {
          Set_Servo_Angle(angle);
          const uint8_t response[] = "\r\nAngle updated\r\n";
          HAL_UART_Transmit(&huart2, (uint8_t *)response,
                            sizeof(response) - 1U, HAL_MAX_DELAY);
        }
        else
        {
          const uint8_t response[] = "\r\nUse 0, 90 or 180\r\n";
          HAL_UART_Transmit(&huart2, (uint8_t *)response,
                            sizeof(response) - 1U, HAL_MAX_DELAY);
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
}

static void MX_TIM4_Init(void)
{
  TIM_OC_InitTypeDef channel_config = {0};

  __HAL_RCC_TIM4_CLK_ENABLE();
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 71;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 19999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }

  channel_config.OCMode = TIM_OCMODE_PWM1;
  channel_config.Pulse = 1500;
  channel_config.OCPolarity = TIM_OCPOLARITY_HIGH;
  channel_config.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &channel_config, TIM_CHANNEL_3) != HAL_OK)
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

static void Set_Servo_Angle(uint32_t angle)
{
  uint32_t pulse = 500U + angle * 2000U / 180U;
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, pulse);
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *timer_handle)
{
  GPIO_InitTypeDef gpio_config = {0};

  if (timer_handle->Instance == TIM4)
  {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    gpio_config.Pin = GPIO_PIN_8;
    gpio_config.Mode = GPIO_MODE_AF_PP;
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
