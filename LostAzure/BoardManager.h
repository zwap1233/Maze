#pragma once

#include <stm32h7xx_hal.h>

typedef struct
{
	uint16_t led_data[109];
	GPIO_TypeDef *SS_PORT;
	uint16_t SS_PIN;
} __packed  board_t;

void initBoardManager();

void writeBoards();
void updateBoards();