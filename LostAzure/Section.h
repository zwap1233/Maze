#pragma once

#include "Maze.h"
#include <stdint.h>

class Room;

#define SECTION_SIZE 9
#define DOOR_ARRAY_SIZE (SECTION_SIZE * 2 + 1) * (SECTION_SIZE + 1)

class Section
{
private:
	friend class Room;
	
	bool doors[DOOR_ARRAY_SIZE];

	int section_x, section_y;
	
public:
	Section(int section_x, int section_y);
	
	Room getRoom(int x, int y);
	bool& getDoor(int x, int y, Direction dir);
	
	Room operator()(int x, int y);
	bool& operator()(int x, int y, Direction dir);
};