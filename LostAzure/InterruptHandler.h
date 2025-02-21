#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers		       */
/*******************************************************************************/
		
	void NMI_Handler(void);
	void HardFault_Handler(void);
	void MemManage_Handler(void);
	void BusFault_Handler(void);
	void UsageFault_Handler(void);
	void SVC_Handler(void);
	void DebugMon_Handler(void);
	void PendSV_Handler(void);
	void SysTick_Handler(void);
	
/******************************************************************************/
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h7xx.s).                    */
/******************************************************************************/	

	void DMA1_Stream0_IRQHandler(void);
	void SPI1_IRQHandler(void);
	void TIM7_IRQHandler(void);
	
/******************************************************************************/
/*						Error Handler					                      */
/******************************************************************************/
	
	void Error_Handler(void);

#ifdef __cplusplus
}
#endif