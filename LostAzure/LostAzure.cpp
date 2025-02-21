/**
  ******************************************************************************
  * @file    FreeRTOS/FreeRTOS_ThreadCreation/Src/main.c
  * @author  MCD Application Team
  * @version V1.2.2
  * @date    25-May-2015
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stm32h7xx_hal.h>

#include <stm32h7xx_ll_dma.h>
#include <stm32h7xx_ll_rcc.h>
#include <stm32h7xx_ll_crs.h>
#include <stm32h7xx_ll_bus.h>
#include <stm32h7xx_ll_system.h>
#include <stm32h7xx_ll_exti.h>
#include <stm32h7xx_ll_cortex.h>
#include <stm32h7xx_ll_utils.h>
#include <stm32h7xx_ll_pwr.h>
#include <stm32h7xx_ll_spi.h>
#include <stm32h7xx_ll_gpio.h>

#include <stdio.h>

#include "app_azure_rtos_config.h"
#include "app_threadx.h"

#include "InterruptHandler.h"
#include "mainThread.h"

static UINT App_ThreadX_Init(VOID *memory_ptr);

static UCHAR tx_byte_pool_buffer[TX_APP_MEM_POOL_SIZE];
static TX_BYTE_POOL tx_app_byte_pool;

TX_THREAD mainThreadHandle;

VOID tx_application_define(VOID *first_unused_memory)
{
	VOID *memory_ptr;

	if (tx_byte_pool_create(&tx_app_byte_pool, const_cast<char*>("Tx App memory pool"), tx_byte_pool_buffer, TX_APP_MEM_POOL_SIZE) != TX_SUCCESS)
	{
		Error_Handler();
	}
	else
	{
		memory_ptr = (VOID *)&tx_app_byte_pool;

		if (App_ThreadX_Init(memory_ptr) != TX_SUCCESS)
		{
			Error_Handler();
		}
	}

}

static UINT App_ThreadX_Init(VOID *memory_ptr)
{
	UINT ret = TX_SUCCESS;
	TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

	CHAR *pointer;

	/* Allocate the stack for ThreadOne.  */
	if (tx_byte_allocate(byte_pool, (VOID **) &pointer, APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
	{
		ret = TX_POOL_ERROR;
	}

	/* Create mainThread.  */
	if (tx_thread_create(&mainThreadHandle,
		const_cast<char*>("mainThread"),
		mainThread,
		0,
		pointer,
		APP_STACK_SIZE,
		THREAD_ONE_PRIO,
		THREAD_ONE_PREEMPTION_THRESHOLD,
		DEFAULT_TIME_SLICE,
		TX_AUTO_START) != TX_SUCCESS)
	{
		ret = TX_THREAD_ERROR;
	}

	return ret;
}

int g_TickCount;

void TickTest()
{
	g_TickCount++;
}

void SystemClock_Config(void);
static void MX_SPI1_Init(void);
static void MX_DMA_Init(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	
	/* Enable I-Cache---------------------------------------------------------*/
	SCB_EnableICache();

	/* Enable D-Cache---------------------------------------------------------*/
	SCB_EnableDCache();
	
	/* STM32F4xx HAL library initialization:
	     - Configure the Flash prefetch, instruction and Data caches
	     - Configure the Systick to generate an interrupt each 1 msec
	     - Set NVIC Group Priority to 4
	     - Global MSP (MCU Support Package) initialization
	*/
	HAL_Init();  
	
	SystemClock_Config();
	
	MX_SPI1_Init();
	MX_DMA_Init();
	
	/* Start scheduler */
	tx_kernel_enter();

	/* We should never get here as control is now taken by the scheduler */
	for (;;) ;
}

void SystemClock_Config(void)
{
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
	while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_4)
	{
	}
	LL_PWR_ConfigSupply(LL_PWR_LDO_SUPPLY);
	LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE0);
	while (LL_PWR_IsActiveFlag_VOS() == 0)
	{
	}
	LL_RCC_HSI_Enable();

	/* Wait till HSI is ready */
	while (LL_RCC_HSI_IsReady() != 1)
	{

	}
	LL_RCC_HSI_SetCalibTrimming(64);
	LL_RCC_HSI_SetDivider(LL_RCC_HSI_DIV1);
	LL_RCC_PLL_SetSource(LL_RCC_PLLSOURCE_HSI);
	LL_RCC_PLL1P_Enable();
	LL_RCC_PLL1Q_Enable();
	LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_8_16);
	LL_RCC_PLL1_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
	LL_RCC_PLL1_SetM(4);
	LL_RCC_PLL1_SetN(30);
	LL_RCC_PLL1_SetP(2);
	LL_RCC_PLL1_SetQ(3);
	LL_RCC_PLL1_SetR(2);
	LL_RCC_PLL1_Enable();

	/* Wait till PLL is ready */
	while (LL_RCC_PLL1_IsReady() != 1)
	{
	}

	/* Intermediate AHB prescaler 2 when target frequency clock is higher than 80 MHz */
	LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_2);

	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);

	/* Wait till System clock is ready */
	while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL1)
	{

	}
	LL_RCC_SetSysPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_1);
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
	LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_2);
	LL_RCC_SetAPB4Prescaler(LL_RCC_APB4_DIV_2);
	LL_SetSystemCoreClock(240000000);

	/* Update the time base */
	if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK)
	{
		Error_Handler();
	}
}

/************************************************************************************
 *						Peripheral Initalization									*
 ************************************************************************************/

DMA_HandleTypeDef hdma_spi1_tx;
SPI_HandleTypeDef hspi1;

static void MX_SPI1_Init(void)
{
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };
	
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI1;
	PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
	
	__HAL_RCC_DMA1_CLK_ENABLE();
	__HAL_RCC_SPI1_CLK_ENABLE();
	
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 0x0;
	hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
	hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
	hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
	hspi1.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
	hspi1.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
	hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
	hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
	hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
	hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
	hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
	if (HAL_SPI_Init(&hspi1) != HAL_OK)
	{
		Error_Handler();
	}
	
	hdma_spi1_tx.Instance = DMA1_Stream0;
	hdma_spi1_tx.Init.Request = DMA_REQUEST_SPI1_TX;
	hdma_spi1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	hdma_spi1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_spi1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_spi1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_spi1_tx.Init.Mode = DMA_NORMAL;
	hdma_spi1_tx.Init.Priority = DMA_PRIORITY_HIGH;
	hdma_spi1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	if (HAL_DMA_Init(&hdma_spi1_tx) != HAL_OK)
	{
		Error_Handler();
	}

	__HAL_LINKDMA(&hspi1, hdmatx, hdma_spi1_tx);
	
	HAL_NVIC_SetPriority(SPI1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(SPI1_IRQn);
	
}

static void MX_DMA_Init(void)
{

	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Stream0_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

}

/************************************************************************************
 *						Systick														*
 ************************************************************************************/

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM7) {
		HAL_IncTick();
	}
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	 /* Infinite loop */
	while (1)
	{
	}
}
#endif
