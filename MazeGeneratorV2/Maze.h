#pragma once

#include <iostream>

class Room;

enum class Direction : int {
	LEFT,
	UP,
	RIGHT,
	DOWN
};

std::ostream& operator<<(std::ostream& out, const Direction& dir);

class Maze
{
private:

	friend class Room;
	friend std::ostream& operator<<(std::ostream& out, const Maze& maze);

	int size_x = 0;
	int size_y = 0;

	bool* doors;

	bool* generator_visits;

	void generate();

public:
	Maze(int size_x, int size_y);
	~Maze();

	Room operator()(int x, int y);
	const Room operator()(int x, int y) const;

	int getWidth() const;
	int getHeight() const;
};

//std::ostream& operator<<(std::ostream& out, const Maze& maze);

inline int Maze::getWidth() const
{
	return size_x;
}

inline int Maze::getHeight() const
{
	return size_y;
}
