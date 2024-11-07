
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <iostream>
#include <vector>
#include "Draw.h"
#include "Maze.h"
#include "Room.h"

using namespace cv;

void makeRandomChange(Maze& maze) {
    int x = rand() % (maze.getWidth() - 2) + 1;
    int y = rand() % (maze.getHeight() - 2) + 1;
    Direction dir = static_cast<Direction>(rand() % 4);

    std::cout << "{" << x << "," << y << "} " << dir << std::endl;

    maze(x, y)(dir) = true;
}

int main()
{

    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);

    Maze maze(10, 10);

    makeRandomChange(maze);
    makeRandomChange(maze);
    makeRandomChange(maze);

    MarkovChain chain(maze);

    chain.validateMarkovChain();
    Vector dist = chain.getNStepDistribution(setupDistribution(chain, maze, Room(3,3, maze)), 1000);
    Mat image = drawMaze(maze, chain, dist);
    displayImage(image, "Dist");

    waitKey();
}