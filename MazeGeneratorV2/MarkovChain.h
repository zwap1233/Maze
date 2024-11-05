#pragma once

#include <Eigen/Sparse>
#include <Eigen/Dense>
#include <iostream>
#include "Maze.h"
#include "Room.h"

typedef Eigen::SparseMatrix<double> Matrix;
typedef Eigen::Matrix<double, Eigen::Dynamic, 1> Vector;

class MarkovChain
{
private:
	Maze& maze;
	Matrix* mat;
	int* translation_table;
	int nr_states;
	int nr_index;
	int nr_valid_index;

	const double return_prob = 0.1f;
	const double remain_prob = 0.05f;

	void createTranslationTable();
	int convertToIndex(const Room& prev, const Room& cord) const;

	bool isValidStepHelper(const Room& prev, const Room& cord) const;

	void updateRoomInChain(const Room& room);
	void generateMarkovChain();

	void validateRoom(int x, int y);

public:

	MarkovChain(Maze& maze);
	~MarkovChain();

	int getMaxValidIndex();
	int getMaxIndex();
	int getNrStates();

	void validateMarkovChain();

	Vector getOccupancyDistribution(int min_steps = 0, int max_steps = 10000);
	Matrix getNStepMatrix(int n);
	void PrintDistribution(const Vector& dist);

	bool isValidStep(const Room& prev, const Room& cord) const;

	int translateIndex(int index) const;
	int translateFromValidIndex(int valid_index) const;
	int translateIndex(const Room& prev, const Room& cord) const;

	std::pair<Room, Room> convertFromIndex(int index) const;

	double& operator()(const Room& prev, const Room& cord, const Room& next);
	double operator()(const Room& prev, const Room& cord, const Room& next) const;

};

Vector getRoomProbabilityDistribution(MarkovChain& chain, Vector& dist);
Vector setupDistribution(MarkovChain& chain, Maze& maze, Room room);

inline int MarkovChain::getMaxValidIndex()
{
	return nr_valid_index;
}

inline int MarkovChain::getMaxIndex()
{
	return nr_index;
}

inline int MarkovChain::getNrStates()
{
	return nr_states;
}

inline int MarkovChain::convertToIndex(const Room& prev, const Room& cord) const
{

	int state_prev = prev.getState();
	int state = cord.getState();

	if (state_prev < 0 || state_prev >= nr_states || state < 0 || state >= nr_states)
		throw std::domain_error("Out of bounds");

	return nr_states * state + state_prev;
}

inline std::pair<Room, Room> MarkovChain::convertFromIndex(int index) const
{
	int state_prev = index % nr_states;
	Room prev(state_prev % maze.getWidth(), state_prev / maze.getWidth(), maze);
	
	int state = index / nr_states;
	Room cord(state % maze.getWidth(), state / maze.getWidth(), maze);

	return std::pair<Room, Room>(prev, cord);
}

inline bool MarkovChain::isValidStepHelper(const Room& prev, const Room& cord) const
{
#ifdef __RETURN_ENABLED__
	if (prev == cord)
		return true;
#endif
	
	if (!prev.isNeighbor(cord))
		return false;

	Direction dir;
	bool isolated = true;
	for (int i = 0; i < 4; ++i) {
		try {
			if (cord.getNeighbor(static_cast<Direction>(i)) == prev) {
				dir = static_cast<Direction>(i);
				isolated = false;
				break;
			}
		}
		catch (std::domain_error e) {
			//do nothing just skip this direction
		}
	}

	if (isolated)
		return false;

	if (cord(dir))
		return true;

	return false;
}

inline bool MarkovChain::isValidStep(const Room& prev, const Room& cord) const
{
	return (translateIndex(prev, cord) >= 0);
}

inline int MarkovChain::translateFromValidIndex(int valid_index) const
{
	if (valid_index < 0 || valid_index >= nr_valid_index)
		throw std::domain_error("out of bounds");

	for (int i = 0; i < nr_index; ++i) {
		if (translation_table[i] == valid_index)
			return i;
	}

	return -1;
}

inline int MarkovChain::translateIndex(int index) const
{
	if (index < 0 || index >= nr_index)
		throw std::domain_error("Out of bounds");

	return translation_table[index];
}

inline int MarkovChain::translateIndex(const Room& prev, const Room& cord) const
{
	return translation_table[convertToIndex(prev, cord)];
}

inline double& MarkovChain::operator()(const Room& prev, const Room& cord, const Room& next)
{
	if (!isValidStep(prev, cord) || !isValidStep(cord, next))
		throw std::domain_error("Invalid Step");
	
	return mat->coeffRef(translateIndex(prev, cord), translateIndex(cord, next));
}

inline double MarkovChain::operator()(const Room& prev, const Room& cord, const Room& next) const
{
	if (!isValidStep(prev, cord) || !isValidStep(cord, next))
		throw std::domain_error("Invalid Step");
	
	return mat->coeffRef(translateIndex(prev, cord), translateIndex(cord, next));
}
