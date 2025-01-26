#pragma once

#include <stm32h7xx_hal.h>

class Board
{
private:
	GPIO_TypeDef* SS_Port;
	uint16_t SS_Pin;
	
	SPI_HandleTypeDef& hspi1;
	
	uint8_t led_data[218];
	
	void transmitData(uint16_t addr, uint8_t *data, int size);
	void receiveData(uint16_t addr, uint8_t *data, int size);
	
public:
	
	Board(SPI_HandleTypeDef& hspi1, GPIO_TypeDef* SS_Port, uint16_t SS_Pin);
	
	//Top Left is 0,0
	void setLED(int x, int y, uint16_t red, uint16_t green, uint16_t blue);
	
	void writeLEDBuffer();

};