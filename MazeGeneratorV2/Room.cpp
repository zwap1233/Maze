#include "Room.h"

Room::Room(int x, int y, Maze& maze) : x(x), y(y), maze(maze)
{
	if (x >= maze.size_x || y >= maze.size_y)
		throw std::domain_error("Out of bounds");
}

Room::Room(int x, int y, const Maze& maze) : x(x), y(y), maze(const_cast<Maze&>(maze))
{
	if (x >= maze.size_x || y >= maze.size_y)
		throw std::domain_error("Out of bounds");
}

void Room::generatorVisit()
{
	maze.generator_visits[y * maze.size_x + x] = true;
	bool deadend[4] = { false, false, false, false };

	while (!deadend[0] || !deadend[1] || !deadend[2] || !deadend[3]) {
		int dir = rand() % 4;
		switch (dir)
		{
		case 0:
			try {
				if (!(maze(x - 1, y).hasGeneratorVisited())) {
					(*this)(Direction::LEFT) = true;
					maze(x - 1, y).generatorVisit();
				}
			}
			catch (std::domain_error e) { ; }
			deadend[0] = true;
			break;
		case 1:
			try{
				if (!(maze(x, y + 1).hasGeneratorVisited())) {
					(*this)(Direction::UP) = true;
					maze(x, y + 1).generatorVisit();
				}
			}
			catch (std::domain_error e) { ; }
			deadend[1] = true;
			break;
		case 2:
			try{
				if (!(maze(x + 1, y).hasGeneratorVisited())) {
					(*this)(Direction::RIGHT) = true;
					maze(x + 1, y).generatorVisit();
				}
			}
			catch (std::domain_error e) { ; }
			deadend[2] = true;
			break;
		case 3:
			try {
				if (!(maze(x, y - 1).hasGeneratorVisited())) {
					(*this)(Direction::DOWN) = true;
					maze(x, y - 1).generatorVisit();
				}
			}
			catch (std::domain_error e) { ; }
			deadend[3] = true;
			break;
		}
	}
}