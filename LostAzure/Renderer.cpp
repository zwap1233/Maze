#include "Renderer.h"

#include "BoardManager.h"
#include "Section.h"
#include "Room.h"

#define led_offset(x,y) ((4-y)*6 + 36*x)
void setLED(uint8_t *buffer, int x, int y, uint16_t red, uint16_t green, uint16_t blue)
{
	if (x > 4 || x < 0 || y > 4 || y < 0)
		return; //out of bounds
	
	buffer[led_offset(x, y) + 2] = (uint8_t)(red & 0xff);
	buffer[led_offset(x, y) + 3] = (uint8_t)(red >> 8);
	buffer[led_offset(x, y) + 4] = (uint8_t)(green & 0xff);
	buffer[led_offset(x, y) + 5] = (uint8_t)(green >> 8);
	buffer[led_offset(x, y) + 6] = (uint8_t)(blue & 0xff);
	buffer[led_offset(x, y) + 7] = (uint8_t)(blue >> 8);
}

void renderRoom(int x, int y, uint8_t *buffer, Section& section)
{
	Room room = section(x, y); //get the desired room
	
	for (int room_x = 0; room_x < 5; room_x++)
	{
		
	}
}

void testPattern(board_t *boards)
{
	double t = (static_cast<double>(HAL_GetTick()) / 1000.0);
	
	for (int b_y = 0; b_y < 3; b_y++)
	{
		for (int b_x = 0; b_x < 3; b_x++)
		{
			for (int y = 0; y < 5; y++)
			{
				for (int x = 0; x < 5; x++)
				{
					uint16_t red = static_cast<uint16_t>(65534*(0.5*sin(2.0 * M_PI * 0.5 * (t + 0.2*(x+5*b_x))) + 0.5));
					uint16_t green = static_cast<uint16_t>(65534*(0.5*sin(2.0 * M_PI * 0.75 * (t + 0.2*(x + 5*b_x))) + 0.5));
					uint16_t blue = static_cast<uint16_t>(65534*(0.5*sin(2.0 * M_PI * 1 *(t + 0.2*(x + 5*b_x))) + 0.5));
					setLED(boards[3*b_y+b_x].led_data, x, y, red, green, blue);
				}
		
			}
		}
	}
}

