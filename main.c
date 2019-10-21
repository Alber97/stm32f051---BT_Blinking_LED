#include "main.h"
#include <string.h>
#include "stm32f0xx_hal.h"
#include "stm32f0xx_it.h"

UART_HandleTypeDef huart1;

int i = 0;
char buff[100];
int len;

int rxIndx;
char rxData[2];
char rxBuff[20];
char transferCplt;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

#ifdef __GNUC__
	#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
	#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
	HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, 100);
	return ch;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	uint8_t i;
	if(huart -> Instance == USART1) //aktualny UART
	{
		if(rxIndx == 0)
		{
			for(i = 0; i < 20; i++) rxBuff[i] = 0; // czyszczenie buforu rx
		}
		if(rxData[0] != 64)
		{
			rxBuff[rxIndx++] = rxData[0]; // dodawanie danych do bufora
		}
		else
		{
			rxIndx = 0;
			transferCplt = 1; // transfer skończony, dane gotowe do odczytu
		}
		HAL_UART_Receive_IT(&huart1, rxData, 1); // aktywowanie przerwania uart dla każdego 1 bajta
	}
}

void send_string(char* s)
{
  HAL_UART_Transmit(&huart1, (uint8_t*)s, strlen(s), 1000);
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  HAL_UART_Receive_IT(&huart1, rxData, 1); // aktywowanie przerwania uart dla każdego 1 bajta
  while(1)
  {
	  if(transferCplt)
	  {
		  sprintf(buff, "Instrukcja: %s \r\n", rxBuff);
		  len = strlen(buff);
		  printf("Odebrano polecenie: \n");
		  HAL_UART_Transmit(&huart1, buff, len, 1000);
		  transferCplt = 0; // reset transferu
		  if((rxBuff[0] == 'b') && (rxBuff[1] == '1'))
		  {
			  HAL_GPIO_WritePin(GPIOC, LD4_Pin, GPIO_PIN_SET);
		  }
		  else if((rxBuff[0] == 'b') && (rxBuff[1] == '0'))
		  {
			  HAL_GPIO_WritePin(GPIOC, LD4_Pin, GPIO_PIN_RESET);
		  }
		  else if((rxBuff[0] == 'g') && (rxBuff[1] == '1'))
		  {
			  HAL_GPIO_WritePin(GPIOC, LD3_Pin, GPIO_PIN_SET);
		  }
		  else if((rxBuff[0] == 'g') && (rxBuff[1] == '0'))
		  {
			  HAL_GPIO_WritePin(GPIOC, LD3_Pin, GPIO_PIN_RESET);
		  }
		  else
		  {
			  sprintf(buff, "Błędna instrukcja...");
			  HAL_UART_Transmit(&huart1, buff, len, 1000);
		  }
		  HAL_Delay(500);
		  i++;
	  }
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOC, LD4_Pin|LD3_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LD4_Pin|LD3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}

void Error_Handler(void)
{

}

#ifdef  USE_FULL_ASSERT

void assert_failed(char *file, uint32_t line)
{ 

}
#endif
