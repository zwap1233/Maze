#pragma once

#include "Maze.h"

class Room
{
private:
	friend class Maze;

	Maze& maze;

	int getDoorIndex(int x, int y) const;

	bool hasGeneratorVisited();
	void generatorVisit();

public:
	int x, y;

	Room(int x, int y, Maze& maze);
	Room(int x, int y, const Maze& maze);

	Room getNeighbor(Direction dir) const;
	bool isNeighbor(const Room& room) const;

	bool& operator()(const Direction dir);
	const bool& operator()(const Direction dir) const;

	Room& operator=(const Room& other);

	bool operator==(const Room& other) const;
	bool operator<(const Room& other) const;
	bool operator>(const Room& other) const;

	int getState() const;
};

inline Room Room::getNeighbor(Direction dir) const
{
	int res_x = x, res_y = y;

	switch (dir)
	{
	case Direction::LEFT:
		res_x = x - 1;
		break;
	case Direction::UP:
		res_y = y + 1;
		break;
	case Direction::RIGHT:
		res_x = x + 1;
		break;
	case Direction::DOWN:
		res_y = y - 1;
		break;
	default:
		break;
	}

	if (res_x >= maze.size_x || res_y >= maze.size_y)
		throw std::domain_error("Out of bounds");

	return Room(res_x, res_y, maze);
}

inline bool Room::isNeighbor(const Room& room) const
{
	return ((abs(x - room.x) == 1) && (abs(y - room.y) == 0))
		|| ((abs(x - room.x) == 0) && (abs(y - room.y) == 1));
}

inline bool Room::operator==(const Room& other) const
{
	return this->getState() == other.getState();
}

inline bool Room::operator<(const Room& other) const
{
	return this->getState() < other.getState();
}

inline bool Room::operator>(const Room& other) const
{
	return this->getState() > other.getState();
}

inline int Room::getDoorIndex(int x, int y) const
{
	return y * (maze.size_x + 1) + x;
}

inline bool Room::hasGeneratorVisited()
{
	if (x < 0 || x >= maze.size_x || y < 0 || y >= maze.size_y)
		return true;

	return maze.generator_visits[y * maze.size_x + x];
}

inline bool& Room::operator()(const Direction dir)
{
	switch (dir)
	{
	case Direction::LEFT:
		//std::cout << "LEFT:" << getIndex(y * 2 + 1, x) << std::endl;
		return maze.doors[getDoorIndex(x, y * 2 + 1)];
	case Direction::UP:
		//std::cout << "UP:" << getIndex(y * 2 + 2, x + 1) << std::endl;
		return maze.doors[getDoorIndex(x + 1, y * 2 + 2)];
	case Direction::RIGHT:
		//std::cout << "RIGHT:" << getIndex(y * 2 + 1, x + 1) << std::endl;
		return maze.doors[getDoorIndex(x + 1, y * 2 + 1)];
	case Direction::DOWN:
		//std::cout << "DOWN:" << getIndex(y * 2, x + 1) << std::endl;
		return maze.doors[getDoorIndex(x + 1, y * 2)];
	default:
		throw std::domain_error("Invalid Direction");
		break;
	}
}

inline const bool& Room::operator()(const Direction dir) const
{
	switch (dir)
	{
	case Direction::LEFT:
		//std::cout << "LEFT:" << getIndex(y * 2 + 1, x) << std::endl;
		return maze.doors[getDoorIndex(x, y * 2 + 1)];
	case Direction::UP:
		//std::cout << "UP:" << getIndex(y * 2 + 2, x + 1) << std::endl;
		return maze.doors[getDoorIndex(x + 1, y * 2 + 2)];
	case Direction::RIGHT:
		//std::cout << "RIGHT:" << getIndex(y * 2 + 1, x + 1) << std::endl;
		return maze.doors[getDoorIndex(x + 1, y * 2 + 1)];
	case Direction::DOWN:
		//std::cout << "DOWN:" << getIndex(y * 2, x + 1) << std::endl;
		return maze.doors[getDoorIndex(x + 1, y * 2)];
	default:
		throw std::domain_error("Invalid Direction");
		break;
	}
}

inline Room& Room::operator=(const Room& other)
{
	x = other.x;
	y = other.y;

	maze = other.maze;

	return *this;
}

inline int Room::getState() const
{
	return y * maze.getWidth() + x;
}

