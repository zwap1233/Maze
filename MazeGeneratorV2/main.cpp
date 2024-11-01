#include <iostream>
#include "Maze.h"
#include "Room.h"
#include "MarkovChain.h"

int main()
{
    Maze maze(15, 10);

    MarkovChain chain(maze);

    chain.validateMarkovChain();
    auto pi = chain.getStationairyDistribution();
    chain.PrintDistribution(pi);
}