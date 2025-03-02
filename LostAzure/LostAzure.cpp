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
void testMemTransfer();
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
	
	MX_DMA_Init();
	MX_SPI1_Init();
	
	testMemTransfer();
	
	/* Start scheduler */
	tx_kernel_enter();

	/* We should never get here as control is now taken by the scheduler */
	for (;;) ;
}

void testMemTransfer()
{
	
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
	LL_RCC_PLL1_SetN(60);
	LL_RCC_PLL1_SetP(2);
	LL_RCC_PLL1_SetQ(6);
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
	LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_2);
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
	LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_2);
	LL_RCC_SetAPB4Prescaler(LL_RCC_APB4_DIV_2);
	LL_SetSystemCoreClock(480000000);

	/* Update the time base */
	if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK)
	{
		Error_Handler();
	}
}

/************************************************************************************
 *						Peripheral Initalization									*
 ************************************************************************************/

static void MX_SPI1_Init(void)
{
	
	LL_SPI_InitTypeDef SPI_InitStruct = { 0 };
	LL_GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	
	LL_RCC_SetSPIClockSource(LL_RCC_SPI123_CLKSOURCE_PLL1Q);
	
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
	//enable DMA clk?
	
	/* SPI1_TX Init */
	LL_DMA_SetPeriphRequest(DMA1, LL_DMA_STREAM_0, LL_DMAMUX1_REQ_SPI1_TX);
	LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_0, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
	LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_0, LL_DMA_PRIORITY_HIGH);
	LL_DMA_SetMode(DMA1, LL_DMA_STREAM_0, LL_DMA_MODE_NORMAL);
	LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_0, LL_DMA_PERIPH_NOINCREMENT);
	LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_0, LL_DMA_MEMORY_INCREMENT);
	LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_0, LL_DMA_PDATAALIGN_HALFWORD);
	LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_0, LL_DMA_MDATAALIGN_HALFWORD);
	LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_0);
//	LL_DMA_EnableFifoMode(DMA1, LL_DMA_STREAM_0);
	LL_DMA_SetFIFOThreshold(DMA1, LL_DMA_STREAM_0, LL_DMA_FIFOTHRESHOLD_1_2);
//	LL_DMA_SetMemoryBurstxfer(DMA1, LL_DMA_STREAM_0, LL_DMA_MBURST_INC4);
//	LL_DMA_SetPeriphBurstxfer(DMA1, LL_DMA_STREAM_0, LL_DMA_PBURST_INC4);
	
	SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
	SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
	SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
	SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
	SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
	SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
	SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV256;
	SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
	SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
	SPI_InitStruct.CRCPoly = 0x0;
	LL_SPI_Init(SPI1, &SPI_InitStruct);
	LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);
	LL_SPI_SetFIFOThreshold(SPI1, LL_SPI_FIFO_TH_01DATA);
//	LL_SPI_EnableNSSPulseMgt(SPI1);

	//Enable Interrupt
	NVIC_SetPriority(SPI1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
	NVIC_EnableIRQ(SPI1_IRQn);
	
	LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);
	/**SPI1 GPIO Configuration
	PA5   ------> SPI1_SCK
	PA6   ------> SPI1_MISO
	PA7   ------> SPI1_MOSI
	*/
	GPIO_InitStruct.Pin = LL_GPIO_PIN_5 | LL_GPIO_PIN_6 | LL_GPIO_PIN_7;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	LL_SPI_EnableGPIOControl(SPI1);
}

static void MX_DMA_Init(void)
{

	/* Init with LL driver */
	/* DMA controller clock enable */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

	/* DMA interrupt init */
	/* DMA1_Stream0_IRQn interrupt configuration */
	NVIC_SetPriority(DMA1_Stream0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
	NVIC_EnableIRQ(DMA1_Stream0_IRQn);

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
