#include "InterruptHandler.h"

#include <stm32h7xx_hal.h>
#include <stm32h7xx_ll_spi.h>

void TransmissionCompletedCallback();

extern TIM_HandleTypeDef htim7;

/*******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers		       */
/*******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
	while (1)
	{
		;
	}
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
	while (1)
	{
		;
	}
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
	while (1)
	{
		;
	}
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
	while (1)
	{
		;
	}
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
	while (1)
	{
		;
	}
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
}

/******************************************************************************/
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h7xx.s).                    */
/******************************************************************************/

void DMA1_Stream0_IRQHandler(void)
{
	
}

void SPI1_IRQHandler(void)
{
	if (LL_SPI_IsActiveFlag_EOT(SPI1))
	{
		LL_SPI_ClearFlag_EOT(SPI1);
		TransmissionCompletedCallback();
	}
}

void TIM7_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim7);
}

/******************************************************************************/
/*						Error Handler					                      */
/******************************************************************************/

void Error_Handler(void)
{
	__disable_irq();
	while (1)
	{
		;
	}
}