#include "BoardManager.h"

#include "InterruptHandler.h"
#include "Renderer.h"

#include <stm32h7xx_ll_dma.h>
#include <stm32h7xx_ll_rcc.h>
#include <stm32h7xx_ll_spi.h>
#include <stm32h7xx_ll_gpio.h>
#include <stm32h7xx_ll_bus.h>
#include <stm32h7xx_ll_utils.h>

#include <math.h>
#include <cstdint>

void configBoard(board_t &board);

#define VSYNC_Port GPIOA
#define VSYNC_Pin LL_GPIO_PIN_3
__aligned(4) board_t board[9] = {
	{ .SS_PORT = GPIOF, .SS_PIN = LL_GPIO_PIN_0 },
	{ .SS_PORT = GPIOF, .SS_PIN = LL_GPIO_PIN_1 },
	{ .SS_PORT = GPIOF, .SS_PIN = LL_GPIO_PIN_2 },
	{ .SS_PORT = GPIOF, .SS_PIN = LL_GPIO_PIN_3 },
	{ .SS_PORT = GPIOF, .SS_PIN = LL_GPIO_PIN_4 },
	{ .SS_PORT = GPIOF, .SS_PIN = LL_GPIO_PIN_5 },
	{ .SS_PORT = GPIOF, .SS_PIN = LL_GPIO_PIN_6 },
	{ .SS_PORT = GPIOF, .SS_PIN = LL_GPIO_PIN_7 },
	{ .SS_PORT = GPIOF, .SS_PIN = LL_GPIO_PIN_8 },
	};

void inline pulseVSYNC()
{
	LL_GPIO_ResetOutputPin(VSYNC_Port, VSYNC_Pin);
	HAL_Delay(1);
	LL_GPIO_SetOutputPin(VSYNC_Port, VSYNC_Pin);
}

void transmitData(board_t &board, uint16_t addr, uint8_t *data, int size)
{
	data[0] = (uint8_t)(addr >> 2);
	data[1] = (uint8_t)(addr & 0b11) << 6;
	data[1] = data[1] | (0b1 << 5);
	
	LL_GPIO_ResetOutputPin(board.SS_PORT, board.SS_PIN);
	
	LL_SPI_DisableDMAReq_TX(SPI1);
	LL_SPI_DisableIT_EOT(SPI1);
	
	LL_SPI_SetTransferDirection(SPI1, LL_SPI_SIMPLEX_TX);
	LL_SPI_SetTransferSize(SPI1, size);
	LL_SPI_SetDataWidth(SPI1, LL_SPI_DATAWIDTH_8BIT);
	LL_SPI_SetFIFOThreshold(SPI1, LL_SPI_FIFO_TH_01DATA);
	
	LL_SPI_Enable(SPI1);
	LL_SPI_StartMasterTransfer(SPI1);
	
	for (int i = 0; i < size; i++)
	{
		while (!LL_SPI_IsActiveFlag_TXP(SPI1)) { ; }
		LL_SPI_TransmitData8(SPI1, data[i]);
	}
	
	while (!LL_SPI_IsActiveFlag_EOT(SPI1)) { ; } //wait till SPI is done
	
	LL_SPI_ClearFlag_EOT(SPI1);
	LL_SPI_ClearFlag_TXTF(SPI1);
	
	LL_SPI_Disable(SPI1);
	
	LL_GPIO_SetOutputPin(board.SS_PORT, board.SS_PIN);
}

void transmitData16(board_t &board, uint16_t addr, uint16_t *data, int size)
{
	data[0] = addr << 6 | (0b1 << 5);
	data[0] = __builtin_bswap16(data[0]);
		
	LL_GPIO_ResetOutputPin(board.SS_PORT, board.SS_PIN);
	
	LL_SPI_DisableDMAReq_TX(SPI1);
	
	LL_SPI_SetTransferDirection(SPI1, LL_SPI_SIMPLEX_TX);
	LL_SPI_SetTransferSize(SPI1, size << 1); //size is in uint16_t while tsize needs to be in uint8_t
	LL_SPI_SetDataWidth(SPI1, LL_SPI_DATAWIDTH_8BIT);
	LL_SPI_SetFIFOThreshold(SPI1, LL_SPI_FIFO_TH_02DATA);
	
	LL_SPI_Enable(SPI1);
	LL_SPI_StartMasterTransfer(SPI1);
	
	for (int i = 0; i < size; i++)
	{
		while (!LL_SPI_IsActiveFlag_TXP(SPI1));
		LL_SPI_TransmitData16(SPI1, data[i]);
	}
	
	while (!LL_SPI_IsActiveFlag_EOT(SPI1)); //wait till SPI is done
	
	LL_SPI_ClearFlag_EOT(SPI1);
	LL_SPI_ClearFlag_TXTF(SPI1);
	
	LL_SPI_Disable(SPI1);
	
	LL_GPIO_SetOutputPin(board.SS_PORT, board.SS_PIN);
}

void transmitDMA(board_t &board, uint16_t addr, uint16_t *data, int size)
{
	data[0] = addr << 6 | (0b1 << 5);
	data[0] = __builtin_bswap16(data[0]); // need to swap the bytes so the MSB byte goes first for address not needed later
	
	//Causes hard fault if D cache is disabled
	SCB_CleanDCache_by_Addr((uint32_t*) data, size << 2); //need to flush Cache for DMA, only does so in 32 BYTE blocks and size in number of bytes
	
	LL_DMA_ConfigAddresses(DMA1,
		LL_DMA_STREAM_0,
		reinterpret_cast<uint32_t>(data),
		LL_SPI_DMA_GetTxRegAddr(SPI1),
		LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
	
	LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_0, size);
	
	LL_SPI_SetTransferDirection(SPI1, LL_SPI_SIMPLEX_TX);
	LL_SPI_SetTransferSize(SPI1, size << 1);
	LL_SPI_SetDataWidth(SPI1, LL_SPI_DATAWIDTH_8BIT);
	LL_SPI_SetFIFOThreshold(SPI1, LL_SPI_FIFO_TH_02DATA);
	
	LL_SPI_EnableDMAReq_TX(SPI1);
	LL_SPI_EnableIT_EOT(SPI1);
	
	LL_SPI_Enable(SPI1);
	while (!LL_SPI_IsEnabled(SPI1)) ; //wait for SPI to enable
	LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_0);
	
	LL_GPIO_ResetOutputPin(board.SS_PORT, board.SS_PIN);
	
	LL_SPI_StartMasterTransfer(SPI1);
	
}

uint8_t completed = 0;

void TransmissionCompletedCallback()
{
	
	LL_GPIO_SetOutputPin(GPIOF, LL_GPIO_PIN_ALL);
	
	LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_0);
	while (LL_DMA_IsEnabledStream(DMA1, LL_DMA_STREAM_0)) ; //wait for DMA stream to disable
	
	LL_SPI_Disable(SPI1);
	
	LL_SPI_DisableIT_EOT(SPI1);
	
	LL_DMA_ClearFlag_FE0(DMA1);
	LL_DMA_ClearFlag_HT0(DMA1);
	LL_DMA_ClearFlag_TC0(DMA1);
	
	LL_SPI_ClearFlag_TXTF(SPI1);
	
	completed = 1;
}

void initBoardManager()
{
	
	//SPI Pins configured by main()
	/**SPI1 GPIO Configuration
	PA5   ------> SPI1_SCK
	PA6   ------> SPI1_MISO
	PA7   ------> SPI1_MOSI
	*/
	
	LL_GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	
	//VSYNC
	LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);
	GPIO_InitStruct.Pin = VSYNC_Pin;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	LL_GPIO_Init(VSYNC_Port, &GPIO_InitStruct);
	
	//SS
	LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOF);
	GPIO_InitStruct.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2 | LL_GPIO_PIN_3 | LL_GPIO_PIN_4 | LL_GPIO_PIN_5 | LL_GPIO_PIN_6 | LL_GPIO_PIN_7 | LL_GPIO_PIN_8;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	LL_GPIO_Init(GPIOF, &GPIO_InitStruct);
	
	LL_GPIO_WriteOutputPort(GPIOF, 0xff);
	
	for (int i = 0; i < 9; i++)
	{
		configBoard(board[i]);
	}
}

void configBoard(board_t &board)
{
	uint8_t data_enable[3];
	data_enable[2] = 1;
	transmitData(board, 0x0, data_enable, 3);
	HAL_Delay(1);
	
	uint8_t data_reset[3];
	data_reset[2] = 0xff;
	transmitData(board, 0xa9, data_reset, 3);
	HAL_Delay(1);
	
	//write dev registers including MC + deghosting
	uint8_t data_dev[6];
	data_dev[2] =  (5 << 3) | (0x2 << 1); //dev_initial max_line_num = 5, data_ref_mode = 3 (16bit + vsync), PWM_Fre = 0 (125kHz)
	data_dev[3] = 0x1 << 2; //dev_config1 SW_BLK = 0, PWM_Scale_Mode = 1 (exponential), PWM_Phase_Shift = 0, CS_ON_Shift = 0
	data_dev[4] = 0x3; //dev_config2 Comp_Group1 = 0, Comp_Group2 = 0, Comp_Group3 = 0, LOD_removal = 1, LSD_removal = 1
	data_dev[5] = 0x4b; // dev_config3 = defaults + MC =0b101 -> 30mA
	transmitData(board, 0x1, data_dev, 6);
	
	//write chip enable to 0 and back to 1 to refresh MC and deghosting settings
	data_enable[2] = 0;
	transmitData(board, 0x0, data_enable, 3);
	HAL_Delay(1);
	data_enable[2] = 1;
	transmitData(board, 0x0, data_enable, 3);
	HAL_Delay(1);

	//configure dot groups?
	
	//disable unconnected dots
	uint8_t data_dots[20];
	data_dots[2] = 0xff; //Dot_onoff0
	data_dots[3] = 0x7f; //Dot_onoff1
	data_dots[4] = 0x0; //Dot_onoff2
	data_dots[5] = 0xff; //Dot_onoff3
	data_dots[6] = 0x7f; //Dot_onoff4
	data_dots[7] = 0x0; //Dot_onoff5
	data_dots[8] = 0xff; //Dot_onoff6
	data_dots[9] = 0x7f; //Dot_onoff7
	data_dots[10] = 0x0; //Dot_onoff8
	data_dots[11] = 0xff; //Dot_onoff9
	data_dots[12] = 0x7f; //Dot_onoff10
	data_dots[13] = 0x0; //Dot_onoff11
	data_dots[14] = 0xff; //Dot_onoff12
	data_dots[15] = 0x7f; //Dot_onoff13
	data_dots[16] = 0x0; //Dot_onoff14
	data_dots[17] = 0x0; //Dot_onoff15
	data_dots[18] = 0x0; //Dot_onoff16
	data_dots[19] = 0x0; //Dot_onoff17
	transmitData(board, 0x43, data_dots, 20);
}

void writeBoards()
{
	for (int i = 0; i < 9; i++)
	{
		transmitDMA(board[i], 0x200, (uint16_t *) board[i].led_data, 109);
//		transmitData(board[i], 0x200, board[i].led_data, 218);
		while (!completed)
		{
			; //wait
		}
		completed = 0;
	}
	
	pulseVSYNC();
}

void updateBoards()
{
	testPattern(board);
	writeBoards();
}