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
	bool rolled[4] = { false, false, false, false };

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
				else if (!rolled[0] && !((x-1) < 0 || (x-1) >= maze.size_x || (y) < 0 || (y) >= maze.size_y)) {
					rolled[0] = true;
					bool open = (rand() % 8) == 0;
					if(open)
						(*this)(Direction::LEFT) = true;
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
				else if (!rolled[1] && !((x) < 0 || (x) >= maze.size_x || (y+1) < 0 || (y+1) >= maze.size_y)) {
					rolled[1] = true;
					bool open = (rand() % 8) == 0;
					if (open)
						(*this)(Direction::LEFT) = true;
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
				else if (!rolled[2] && !((x + 1) < 0 || (x + 1) >= maze.size_x || (y) < 0 || (y) >= maze.size_y)) {
					rolled[2] = true;
					bool open = (rand() % 8) == 0;
					if (open)
						(*this)(Direction::LEFT) = true;
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
				else if (!rolled[3] && !((x) < 0 || (x) >= maze.size_x || (y-1) < 0 || (y-1) >= maze.size_y)) {
					rolled[3] = true;
					bool open = (rand() % 8) == 0;
					if (open)
						(*this)(Direction::LEFT) = true;
				}
			}
			catch (std::domain_error e) { ; }
			deadend[3] = true;
			break;
		}
	}
}