#include "MarkovChain.h"

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
				(*this)(from, room, to) = (1.0f - remain_prob) / static_cast<float>(neighbors.size() - 1);
				continue;
			}
#endif

			if (from == to)
				(*this)(from, room, to) = return_prob;
			else
#ifdef __RETURN_ENABLED__
				(*this)(from, room, to) = (1.0f - return_prob - remain_prob) / static_cast<float>(neighbors.size() - 2);
#else
				(*this)(from, room, to) = (1.0f - return_prob) / static_cast<float>(neighbors.size() - 1);
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

Vector MarkovChain::getStationairyDistribution()
{
	Matrix I(mat->rows(), mat->cols());
	I.setIdentity();
	I.makeCompressed();

	Vector Z(mat->rows());
	Z.setZero();

	Vector pi(mat->rows());

	Matrix eq = (*mat) - I;
	eq.makeCompressed();
	std::cout << *mat;
	std::cout << eq;
	std::cout << "is row major? " << eq.IsRowMajor << std::endl;

	Eigen::SparseQR<Matrix, Eigen::COLAMDOrdering<int>> solver;
	solver.compute(eq);
	if (solver.info() != Eigen::Success) {
		std::cout << "failure " << solver.info() << std::endl;
	}

	pi = solver.solve(Z);
	if (solver.info() != Eigen::Success) {
		std::cout << "failure2" << std::endl;
	}

	return pi;
}

Matrix MarkovChain::nStepMat(int n)
{
	if (n <= 0)
		throw std::invalid_argument("n must be bigger than 0");

	if (n == 1)
		return *mat;

	Matrix res = (*mat) * (*mat);

	if (n == 2)
		return res;

	for (int i = 2; i < n; ++i) {
		res = res * (*mat);
	}

	return res;
}


void MarkovChain::PrintDistribution(const Vector& dist) {
	for (int i = 0; i < dist.rows(); ++i) {
		int index = translateFromValidIndex(i);
		auto ind = convertFromIndex(index);
		std::cout << "Valid Index: " << i << " Index: " << index << " Value: " << dist(i) << " {{" << ind.first.x << "," << ind.first.y << "},{" << ind.second.x << "," << ind.second.y << "}} " << std::endl;
	}
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
		float x = res(i);
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