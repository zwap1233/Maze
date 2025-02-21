#include "Section.h"
#include "Room.h"
#include <stdlib.h>
#include <cstring>

Section::Section(int section_x, int section_y)
	: section_x(section_x)
	, section_y(section_y)
{
	std::memset(doors, 0, DOOR_ARRAY_SIZE);
}

Room Section::getRoom(int x, int y)
{
	return Room(*this, x, y);
}

Room Section::operator()(int x, int y)
{
	return getRoom(x, y);
}

bool& Section::getDoor(int x, int y, Direction dir)
{
	int _x = x - section_x;
	int _y = y - section_y;
	
	switch (dir)
	{
	case Direction::LEFT:
		return doors[((_y * 2 + 1) * (SECTION_SIZE + 1)) + _x];
	case Direction::UP:
		return doors[(_y * 2 * (SECTION_SIZE + 1)) + _x + 1];
	case Direction::RIGHT:
		return doors[((_y * 2 + 1) * (SECTION_SIZE + 1)) + _x + 1];
	case Direction::DOWN:
		return doors[(_y * 2 * (SECTION_SIZE + 1)) + _x + 1];
	}
}

bool& Section::operator()(int x, int y, Direction dir)
{
	return getDoor(x, y, dir);
}