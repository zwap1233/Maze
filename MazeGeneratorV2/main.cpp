#include <iostream>
#include "Maze.h"
#include "Room.h"
#include "MarkovChain.h"

int main()
{
    Maze maze(2, 2);

    MarkovChain chain(maze);

    chain.validateMarkovChain();
    auto pi = chain.getStationairyDistribution();
    chain.PrintDistribution(pi);

    //std::cout << (chain.nStepMat(20)-chain.nStepMat(19)) << std::endl;
}