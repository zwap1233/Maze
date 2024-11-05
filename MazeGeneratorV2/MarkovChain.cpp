#include "MarkovChain.h"
#include "Draw.h"

MarkovChain::MarkovChain(Maze& maze) : maze(maze)
{
	nr_states = (maze.getWidth() * maze.getHeight());
	nr_index = nr_states * nr_states;

	createTranslationTable();

	mat = new Matrix(nr_valid_index, nr_valid_index);

	generateMarkovChain();
}

MarkovChain::~MarkovChain()
{
	delete mat;
	delete translation_table;
}

void MarkovChain::generateMarkovChain()
{
	for (int x = 0; x < maze.getWidth(); ++x) {
		for (int y = 0; y < maze.getHeight(); ++y) {
			updateRoomInChain(Room(x, y, maze));
		}
	}

	mat->makeCompressed();
}

void MarkovChain::updateRoomInChain(const Room& room)
{
	std::vector<Room> neighbors;

	for (int i = 0; i < 4; ++i) {
		if (room(static_cast<Direction>(i))) {
			try {
				neighbors.push_back(room.getNeighbor(static_cast<Direction>(i)));
			}
			catch (std::domain_error e) {
				; // do nothing just skip it.
			}
		}
	}

	if (neighbors.empty()) {
		(*this)(room, room, room) = 1.0f;
		return;
	}

	if (neighbors.size() == 1) {
#ifdef __RETURN_ENABLED__
		(*this)(neighbors[0], room, neighbors[0]) = 1.0f - remain_prob;
		(*this)(room, room, neighbors[0]) = 1.0f - remain_prob;
		(*this)(neighbors[0], room, room) = remain_prob;
		(*this)(room, room, room) = remain_prob;
#else
		(*this)(neighbors[0], room, neighbors[0]) = 1.0f;
#endif
		return;
	}

#ifdef __RETURN_ENABLED__
	neighbors.push_back(room);
#endif

	for (const Room from : neighbors) {
		for (const Room to : neighbors) {
#ifdef __RETURN_ENABLED__
			if (to == room) {
				(*this)(from, room, to) = remain_prob;
				continue;
			}

			if (from == room) {
				(*this)(from, room, to) = (1.0f - remain_prob) / static_cast<double>(neighbors.size() - 1);
				continue;
			}
#endif

			if (from == to)
				(*this)(from, room, to) = return_prob;
			else
#ifdef __RETURN_ENABLED__
				(*this)(from, room, to) = (1.0f - return_prob - remain_prob) / static_cast<double>(neighbors.size() - 2);
#else
				(*this)(from, room, to) = (1.0f - return_prob) / static_cast<double>(neighbors.size() - 1);
#endif
		}
	}
}

void MarkovChain::createTranslationTable()
{
	translation_table = new int[nr_index];
	nr_valid_index = 0;
	for (int i = 0; i < nr_index; ++i) {
		auto rooms = convertFromIndex(i);
		if (isValidStepHelper(rooms.first, rooms.second)) {
			translation_table[i] = nr_valid_index;
			++nr_valid_index;
		}
		else {
			translation_table[i] = -1;
		}
	}
}

//Vector MarkovChain::getOccupancyDistribution(int max_steps)
//{
//	Eigen::IOFormat format(7, 0, ", ", "\n", "[", "]", "", "\n");
//
//	Vector dist = setupDistribution(*this, maze, Room(5, 5, maze));
//	std::cout << dist.format(format) << std::endl;
//
//
//	Vector dist_odd = dist;
//	Vector diff(mat->rows());
//	diff.setZero();
//
//	for (int i = 0; i < max_steps; ++i) {
//		dist = (*mat) * dist;
//	}
//	
//	dist_odd = (*mat) * dist;
//	std::cout << dist.format(format) << std::endl;
//	std::cout << dist_odd.format(format) << std::endl;
//	std::cout << (dist - dist_odd).format(format) << std::endl;
//	std::cout << ((dist_odd + dist) / 2).format(format) << std::endl;
//
//
//	return dist;
//}

Vector MarkovChain::getOccupancyDistribution(int min_steps, int max_steps)
{
	//Eigen::IOFormat format(7, 0, ", ", "\n", "[", "]", "", "\n");

	Vector dist_even = setupDistribution(*this, maze, Room(5, 5, maze));

	Vector dist_ref = dist_even;
	Vector dist_odd = dist_even;
	Vector diff(mat->rows());
	diff.setZero();

	for (int i = 0; i < min_steps; ++i) {
		dist_even = (*mat) * dist_even;
	}

	for (int i = 0; i < max_steps; ++i) {
		dist_ref = dist_even;
		dist_odd = (*mat) * dist_even;
		dist_even = (*mat) * dist_odd;
		
		diff = dist_ref - dist_even;

		if (diff.maxCoeff() < 0.0000001) {
			/*std::cout << dist_even.format(format) << std::endl;
			std::cout << dist_ref.format(format) << std::endl;
			std::cout << dist_odd.format(format) << std::endl;
			std::cout << (dist_odd - dist_even).format(format) << std::endl;
			std::cout << ((dist_even + dist_odd) / 2).format(format) << std::endl;*/

			return ((dist_even + dist_odd) / 2);
		}
	}

	dist_even.setZero();
	return dist_even;
}



Matrix MarkovChain::getNStepMatrix(int n)
{
	Matrix P = (*mat);

	if (n < 0)
		throw std::invalid_argument("invalid n");

	if (n == 1)
		return (*mat);

	for (int i = 1; i < n; ++i) {
		P = P * (*mat);
	}

	return P;
}

void MarkovChain::PrintDistribution(const Vector& dist) {
	for (int i = 0; i < dist.rows(); ++i) {
		int index = translateFromValidIndex(i);
		auto ind = convertFromIndex(index);
		std::cout << "Valid Index: " << i << " Index: " << index << " Value: " << dist(i) << " {{" << ind.first.x << "," << ind.first.y << "},{" << ind.second.x << "," << ind.second.y << "}} " << std::endl;
	}
}

Vector getRoomProbabilityDistribution(MarkovChain& chain, Vector& dist)
{
	std::map<Room, double> room_map;

	for (int i = 0; i < dist.size(); ++i) {
		auto cords = chain.convertFromIndex(chain.translateFromValidIndex(i));
		room_map[cords.second] += dist(i);
	}

	Vector res(room_map.size());

	for (auto it = room_map.begin(); it != room_map.end(); ++it) {
		Room room = it->first;
		double value = it->second;
		
		res(room.getState()) = value;
	}

	return res;
}

Vector setupDistribution(MarkovChain& chain, Maze& maze, Room room) {
	Vector dist(chain.getMaxValidIndex());
	std::vector<Room> neighbors;

	dist.setZero();

	if (room(Direction::LEFT)) {
		neighbors.push_back(room.getNeighbor(Direction::LEFT));
	}

	if (room(Direction::UP)) {
		neighbors.push_back(room.getNeighbor(Direction::UP));
	}

	if (room(Direction::RIGHT)) {
		neighbors.push_back(room.getNeighbor(Direction::RIGHT));
	}

	if (room(Direction::DOWN)) {
		neighbors.push_back(room.getNeighbor(Direction::DOWN));
	}

	for (Room from : neighbors) {
		dist(chain.translateIndex(from, room)) = (1.0f / neighbors.size());
	}

	return dist;
}

// ================== Validation Functions ======================

void MarkovChain::validateMarkovChain()
{
	std::cout << "++++++++++++++++++++ Validating Markov Chain ++++++++++++++++++++++++++" << std::endl << std::endl;
	std::cout << maze << std::endl;

	/*std::cout << "============== Room Tests ==============" << std::endl << std::endl;
	validateRoom(0, 0);
	validateRoom(4, 4);
	validateRoom(4, 3);*/

	std::cout << "============== Row Tests ===============" << std::endl << std::endl;

	Vector vec(mat->cols());
	for (int i = 0; i < vec.rows(); ++i) {
		vec(i) = 1.0f;
	}

	auto res = (*mat) * vec;

	int failures = 0;
	for (int i = 0; i < res.rows(); ++i) {
		double x = res(i);
		auto cords = convertFromIndex(translateFromValidIndex(i));
		if (abs(x - 1.0f) > 0.005) {
			std::cout << "Test failed for row " << i << " { From: {" << cords.first.x << "," << cords.first.y << "}, at: {" << cords.second.x << "," << cords.second.y << "}}" << " adds to: " << x << std::endl;
			++failures;
			continue;
		}
		//std::cout << "Test succeeded for row " << i << " { From: {" << cords.first.x << "," << cords.first.y << "}, at: {" << cords.second.x << "," << cords.second.y << "}}" << " adds to: " << x << std::endl;
	}
	std::cout << "Test ended, " << failures << " rows failed" << std::endl << std::endl;

	std::cout << "============== Tests finished ===============" << std::endl << std::endl;
}

void MarkovChain::validateRoom(int x, int y)
{
	std::cout << "---------- Test (" << x << "," << y << ") ------------" << std::endl;
	Room room(x, y, maze);
	Direction dir = Direction::LEFT;
	std::cout << "From LEFT" << std::endl;
	std::cout << "LEFT: "; try { std::cout << (*this)(room.getNeighbor(dir), room, room.getNeighbor(Direction::LEFT)); } catch (std::domain_error e) { std::cout << "0.0"; };
	std::cout << " UP: "; try { std::cout << (*this)(room.getNeighbor(dir), room, room.getNeighbor(Direction::UP)); } catch (std::domain_error e) { std::cout << "0.0"; };
	std::cout << " RIGHT: "; try { std::cout << (*this)(room.getNeighbor(dir), room, room.getNeighbor(Direction::RIGHT)); } catch (std::domain_error e) { std::cout << "0.0"; };
	std::cout << " DOWN: "; try { std::cout << (*this)(room.getNeighbor(dir), room, room.getNeighbor(Direction::DOWN)); } catch (std::domain_error e) { std::cout << "0.0"; }; std::cout << std::endl;

	std::cout << "From UP" << std::endl;
	dir = Direction::UP;
	std::cout << "LEFT: "; try { std::cout << (*this)(room.getNeighbor(dir), room, room.getNeighbor(Direction::LEFT)); } catch (std::domain_error e) { std::cout << "0.0"; };
	std::cout << " UP: "; try { std::cout << (*this)(room.getNeighbor(dir), room, room.getNeighbor(Direction::UP)); } catch (std::domain_error e) { std::cout << "0.0"; };
	std::cout << " RIGHT: "; try { std::cout << (*this)(room.getNeighbor(dir), room, room.getNeighbor(Direction::RIGHT)); } catch (std::domain_error e) { std::cout << "0.0"; };
	std::cout << " DOWN: "; try { std::cout << (*this)(room.getNeighbor(dir), room, room.getNeighbor(Direction::DOWN)); } catch (std::domain_error e) { std::cout << "0.0"; }; std::cout << std::endl;

	std::cout << "From RIGHT" << std::endl;
	dir = Direction::RIGHT;
	std::cout << "LEFT: "; try { std::cout << (*this)(room.getNeighbor(dir), room, room.getNeighbor(Direction::LEFT)); } catch (std::domain_error e) { std::cout << "0.0"; };
	std::cout << " UP: "; try { std::cout << (*this)(room.getNeighbor(dir), room, room.getNeighbor(Direction::UP)); } catch (std::domain_error e) { std::cout << "0.0"; };
	std::cout << " RIGHT: "; try { std::cout << (*this)(room.getNeighbor(dir), room, room.getNeighbor(Direction::RIGHT)); } catch (std::domain_error e) { std::cout << "0.0"; };
	std::cout << " DOWN: "; try { std::cout << (*this)(room.getNeighbor(dir), room, room.getNeighbor(Direction::DOWN)); } catch (std::domain_error e) { std::cout << "0.0"; }; std::cout << std::endl;

	std::cout << "From DOWN" << std::endl;
	dir = Direction::DOWN;
	std::cout << "LEFT: "; try { std::cout << (*this)(room.getNeighbor(dir), room, room.getNeighbor(Direction::LEFT)); } catch (std::domain_error e) { std::cout << "0.0"; };
	std::cout << " UP: "; try { std::cout << (*this)(room.getNeighbor(dir), room, room.getNeighbor(Direction::UP)); } catch (std::domain_error e) { std::cout << "0.0"; };
	std::cout << " RIGHT: "; try { std::cout << (*this)(room.getNeighbor(dir), room, room.getNeighbor(Direction::RIGHT)); } catch (std::domain_error e) { std::cout << "0.0"; };
	std::cout << " DOWN: "; try { std::cout << (*this)(room.getNeighbor(dir), room, room.getNeighbor(Direction::DOWN)); } catch (std::domain_error e) { std::cout << "0.0"; }; std::cout << std::endl;

	std::cout << std::endl;
}
