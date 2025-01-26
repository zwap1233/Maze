#include "Board.h"

#include "app_threadx.h"

Board::Board(SPI_HandleTypeDef& hspi1, GPIO_TypeDef* SS_Port, uint16_t SS_Pin)
		: hspi1(hspi1)
		, SS_Port(SS_Port)
		, SS_Pin(SS_Pin)
{
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = SS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(SS_Port, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(SS_Port, SS_Pin, GPIO_PIN_SET);
	
	//write chip enable to 1, registers can only be written in this setting
	uint8_t data_enable[3];
	data_enable[2] = 1;
	transmitData(0x0, data_enable, 3);
	HAL_Delay(1);
	
	uint8_t data_reset[3];
	data_reset[2] = 0xff;
	transmitData(0xa9, data_reset, 3);
	HAL_Delay(1);
	
	//write dev registers including MC + deghosting
	uint8_t data_dev[6];
	data_dev[2] =  (5 << 3) | (0x2 << 1); //dev_initial max_line_num = 5, data_ref_mode = 3 (16bit + vsync), PWM_Fre = 0 (125kHz)
	data_dev[3] = 0x1 << 2; //dev_config1 SW_BLK = 0, PWM_Scale_Mode = 1 (exponential), PWM_Phase_Shift = 0, CS_ON_Shift = 0
	data_dev[4] = 0x3; //dev_config2 Comp_Group1 = 0, Comp_Group2 = 0, Comp_Group3 = 0, LOD_removal = 1, LSD_removal = 1
	data_dev[5] = 0x4b; // dev_config3 = defaults + MC =0b101 -> 30mA
	transmitData(0x1, data_dev, 6);
	
	//write chip enable to 0 and back to 1 to refresh MC and deghosting settings
	data_enable[2] = 0;
	transmitData(0x0, data_enable, 3);
	HAL_Delay(1);
	data_enable[2] = 1;
	transmitData(0x0, data_enable, 3);
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
	transmitData(0x43, data_dots, 20);
	//write dot current settings
	
	//the chip uses both analog current diming and pwm diming at the same time
}

void Board::transmitData(uint16_t addr, uint8_t *data, int size)
{
	data[0] = (uint8_t)(addr >> 2);
	data[1] = (uint8_t)(addr & 0b11) << 6;
	data[1] = data[1] | (0b1 << 5);
	
	HAL_GPIO_WritePin(SS_Port, SS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, data, size, 100);
	HAL_GPIO_WritePin(SS_Port, SS_Pin, GPIO_PIN_SET);
	
}

void Board::receiveData(uint16_t addr, uint8_t *data, int size)
{
	data[0] = (uint8_t)(addr >> 2);
	data[1] = (uint8_t)(addr & 0b11) << 6;
	data[1] = data[1];
	
	HAL_GPIO_WritePin(SS_Port, SS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, data, 2, 100);
	HAL_SPI_Receive(&hspi1, &(data[2]), size - 2, 100);
	HAL_GPIO_WritePin(SS_Port, SS_Pin, GPIO_PIN_SET);
	
}

#define led_offset(x,y) ((4-y)*6 + 36*x)
void Board::setLED(int x, int y, uint16_t red, uint16_t green, uint16_t blue)
{
	if (x > 4 || x < 0 || y > 4 || y < 0)
		return; //out of bounds
	
	led_data[led_offset(x, y) + 2] = (uint8_t)(red & 0xff);
	led_data[led_offset(x, y) + 3] = (uint8_t)(red >> 8);
	led_data[led_offset(x, y) + 4] = (uint8_t)(green & 0xff);
	led_data[led_offset(x, y) + 5] = (uint8_t)(green >> 8);
	led_data[led_offset(x, y) + 6] = (uint8_t)(blue & 0xff);
	led_data[led_offset(x, y) + 7] = (uint8_t)(blue >> 8);
}
	
void Board::writeLEDBuffer()
{
	transmitData(0x200, led_data, 218);
}