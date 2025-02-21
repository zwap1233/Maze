#pragma once

#include <stm32h7xx_hal.h>

typedef struct
{
	GPIO_TypeDef *SS_PORT;
	uint16_t SS_PIN;
	
	uint8_t led_data[218];
} board_t;

void initBoardManager();

void writeBoards();
void updateBoards();