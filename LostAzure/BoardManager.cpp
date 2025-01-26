#include "BoardManager.h"

#include "Board.h"
#include "InterruptHandler.h"

#include <stm32h7xx_hal.h>

#include <math.h>

extern SPI_HandleTypeDef hspi1;

#define SS_Port GPIOA
#define SS_Pin GPIO_PIN_4

#define VSYNC_Port GPIOA
#define VSYNC_Pin GPIO_PIN_3

#define VIO_EN_Port GPIOA
#define VIO_EN_Pin GPIO_PIN_2

void inline pulseVSYNC()
{
	HAL_GPIO_WritePin(VSYNC_Port, VSYNC_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(VSYNC_Port, VSYNC_Pin, GPIO_PIN_RESET);
}

Board *board;

void initBoardManager()
{
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	//VIO_EN, VSYNC
	GPIO_InitStruct.Pin = VSYNC_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(VSYNC_Port, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = VIO_EN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(VIO_EN_Port, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(VIO_EN_Port, VIO_EN_Pin, GPIO_PIN_SET);
	
	//create Boards
	board = new Board(hspi1, SS_Port, SS_Pin);
}

void updateTestPattern()
{
	double t = (static_cast<double>(HAL_GetTick()) / 1000.0);
	
	for (int y = 0; y < 5; y++)
	{
		for (int x = 0; x < 5; x++)
		{
			uint16_t red = static_cast<uint16_t>(65534*(0.5*sin(2.0 * M_PI * 0.5 * (t + 0.2*x)) + 0.5));
			uint16_t green = static_cast<uint16_t>(65534*(0.5*sin(2.0 * M_PI * 0.75 * (t + 0.2*x)) + 0.5));
			uint16_t blue = static_cast<uint16_t>(65534*(0.5*sin(2.0 * M_PI * 1 *(t + 0.2*x)) + 0.5));
			board->setLED(x, y, red, green, blue);
		}
		
	}
	
	board->writeLEDBuffer();
	
	pulseVSYNC();
}