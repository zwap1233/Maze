#include "Maze.h"
#include "Room.h"

Maze::Maze(int size_x, int size_y) : size_x(size_x), size_y(size_y)
{
	doors = (bool*)calloc((size_y * 2 + 1) * (size_x + 1), sizeof(bool));

	generate();
}

Maze::~Maze()
{
	free(doors);
}

void Maze::generate()
{
	std::srand(std::time(nullptr));
	generator_visits = (bool*) calloc(size_x * size_y, sizeof(bool));
	(*this)(0, 0).generatorVisit();

	free(generator_visits);
}

std::ostream& operator<<(std::ostream& out, const Maze& maze)
{
	out << "Dimensions: " << maze.size_y << "x" << maze.size_x << std::endl;
	for (int y = maze.size_y - 1; y >= 0; --y) {
		for (int x = 0; x < maze.size_x; ++x) {
			if (maze(x,y)(Direction::UP))
				out << "+ ";
			else
				out << "+-";
		}
		out << "+" << std::endl;

		for (int x = 0; x < maze.size_x; ++x) {
			if (maze(x, y)(Direction::LEFT))
				out << "  ";
			else
				out << "| ";
		}

		if (maze(maze.size_x - 1, y)(Direction::RIGHT))
			out << " " << std::endl;
		else
			out << "|" << std::endl;
	}

	for (int x = 0; x < maze.size_x; ++x) {
		if (maze(x, 0)(Direction::DOWN))
			out << "+ ";
		else
			out << "+-";
	}
	out << "+" << std::endl;

	return out;
}

Room Maze::operator()(int x, int y)
{
	return Room(x, y, *this);
}

const Room Maze::operator()(int x, int y) const
{
	return Room(x,y, *this);
}
