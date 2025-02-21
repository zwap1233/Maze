#include "BoardManager.h"

#include "InterruptHandler.h"
#include "Renderer.h"

#include <math.h>

void configBoard(board_t &board);

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_tx;

#define VSYNC_Port GPIOA
#define VSYNC_Pin GPIO_PIN_3

board_t board[9] = {
	{ .SS_PORT = GPIOF, .SS_PIN = GPIO_PIN_0 },
	{ .SS_PORT = GPIOF, .SS_PIN = GPIO_PIN_1 },
	{ .SS_PORT = GPIOF, .SS_PIN = GPIO_PIN_2 },
	{ .SS_PORT = GPIOF, .SS_PIN = GPIO_PIN_3 },
	{ .SS_PORT = GPIOF, .SS_PIN = GPIO_PIN_4 },
	{ .SS_PORT = GPIOF, .SS_PIN = GPIO_PIN_5 },
	{ .SS_PORT = GPIOF, .SS_PIN = GPIO_PIN_6 },
	{ .SS_PORT = GPIOF, .SS_PIN = GPIO_PIN_7 },
	{ .SS_PORT = GPIOF, .SS_PIN = GPIO_PIN_8 },
	};

void inline pulseVSYNC()
{
	HAL_GPIO_WritePin(VSYNC_Port, VSYNC_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(VSYNC_Port, VSYNC_Pin, GPIO_PIN_RESET);
}

void transmitData(board_t &board, uint16_t addr, uint8_t *data, int size)
{
	data[0] = (uint8_t)(addr >> 2);
	data[1] = (uint8_t)(addr & 0b11) << 6;
	data[1] = data[1] | (0b1 << 5);
	
	HAL_GPIO_WritePin(board.SS_PORT, board.SS_PIN, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, data, size, 100);
	HAL_GPIO_WritePin(board.SS_PORT, board.SS_PIN, GPIO_PIN_SET);
}

void receiveData(board_t &board, uint16_t addr, uint8_t *data, int size)
{
	data[0] = (uint8_t)(addr >> 2);
	data[1] = (uint8_t)(addr & 0b11) << 6;
	data[1] = data[1];
	
	HAL_GPIO_WritePin(board.SS_PORT, board.SS_PIN, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, data, 2, 100);
	HAL_SPI_Receive(&hspi1, &(data[2]), size - 2, 100);
	HAL_GPIO_WritePin(board.SS_PORT, board.SS_PIN, GPIO_PIN_SET);	
}

void transmitDMA(board_t &board, uint16_t addr, uint8_t *data, int size)
{
	data[0] = (uint8_t)(addr >> 2);
	data[1] = (uint8_t)(addr & 0b11) << 6;
	data[1] = data[1] | (0b1 << 5);
	
	HAL_GPIO_WritePin(board.SS_PORT, board.SS_PIN, GPIO_PIN_RESET);
	HAL_SPI_Transmit_DMA(&hspi1, data, size);
}


GPIO_TypeDef* current_SS_PORT;
uint16_t current_SS_PIN;
void TransmissionCompletedCallback(SPI_HandleTypeDef* hspi)
{
	//set SS back up
	HAL_GPIO_WritePin(current_SS_PORT, current_SS_PIN, GPIO_PIN_SET);
}

void initBoardManager()
{
	
	//SPI Pins
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	//VSYNC
	GPIO_InitStruct.Pin = VSYNC_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(VSYNC_Port, &GPIO_InitStruct);
	
	//SS
	__HAL_RCC_GPIOF_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
	
	for (int i = 0; i < 9; i++)
	{
		HAL_GPIO_WritePin(board[i].SS_PORT, board[i].SS_PIN, GPIO_PIN_SET);
	}
	
	HAL_SPI_RegisterCallback(&hspi1, HAL_SPI_TX_COMPLETE_CB_ID, TransmissionCompletedCallback);
	
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
		transmitData(board[i], 0x200, board[i].led_data, 218);
	}
	
	pulseVSYNC();
}

void updateBoards()
{
	testPattern(board);
	writeBoards();
}