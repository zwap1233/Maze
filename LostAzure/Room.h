#pragma once

#include "Maze.h"
#include "Section.h"
#include <stdlib.h>

class Room
{
private:
	
	//friend class Section;
	int x, y;
	
	Section& section;
	
public:
	
	Room(Section& section, int x, int y) : section(section), x(x), y(y) {}

	bool isNeighbor(const Room& room) const;
	
	bool& operator()(const Direction dir);
	bool operator()(const Direction dir) const;
	
	Room& operator=(const Room& other);
	
	bool operator==(const Room& other) const;
	bool operator<(const Room& other) const;
	bool operator>(const Room& other) const;
};

inline bool Room::isNeighbor(const Room& room) const
{
	return ((abs(x - room.x) == 1) && (abs(y - room.y) == 0))
	|| ((abs(x - room.x) == 0) && (abs(y - room.y) == 1));
}

inline bool& Room::operator()(const Direction dir)
{
	return section.getDoor(x, y, dir);
}

inline bool Room::operator()(const Direction dir) const
{
	return section.getDoor(x, y, dir);
}

inline Room& Room::operator = (const Room& other)
{
	this->x = other.x;
	this->y = other.y;
	this->section = other.section;
	return *this;
}

inline bool Room::operator==(const Room& other) const
{
	return (this->x == other.x) && (this->y == other.y);
}

inline bool Room::operator<(const Room& other) const
{
	return this < &other;
}

inline bool Room::operator>(const Room& other) const
{
	return this > &other;
}