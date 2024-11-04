
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <vector>
#include "Draw.h"
#include "Maze.h"
#include "Room.h"

using namespace cv;

Vector setupDistribution(MarkovChain& chain, Maze& maze, Room room) {
    Vector dist(chain.getMaxValidIndex());
    std::vector<Room> neighbors;

    if (room(Direction::LEFT)) {
        neighbors.push_back(room.getNeighbor(Direction::LEFT));
        std::cout << "LEFT,";
    }
    
    if (room(Direction::UP)) {
        neighbors.push_back(room.getNeighbor(Direction::UP));
        std::cout << "UP,";
    }
    
    if (room(Direction::RIGHT)) {
        neighbors.push_back(room.getNeighbor(Direction::RIGHT));
        std::cout << "RIGHT,";
    }
    
    if (room(Direction::DOWN)) {
        neighbors.push_back(room.getNeighbor(Direction::DOWN));
        std::cout << "DOWN,";
    }

    for (Room from : neighbors) {
        dist(chain.translateIndex(from, room)) = (1.0f / neighbors.size());
        std::cout << "{" << from.x << "," << from.y << "}" << std::endl;
    }

    return dist;
}

int main()
{
    Maze maze(10, 10);

    MarkovChain chain(maze);

    chain.validateMarkovChain();
    Matrix P = chain.getNStepMatrix(300);
    //Matrix P2 = chain.getNStepMatrix(301);

    Vector ones(P.rows());
    ones.setOnes();
    ones = P * (ones / P.rows());


    Room center(5, 5, maze);
    Vector center_dist = P * setupDistribution(chain, maze, center);

    Room origin(0, 0, maze);
    Vector origin_dist = P * setupDistribution(chain, maze, origin);

    Mat image_ones = drawMaze(maze, chain, ones);
    Mat image_center = drawMaze(maze, chain, center_dist);
    Mat image_origin = drawMaze(maze, chain, origin_dist);
    displayImage(image_ones, "Dist ones");
    displayImage(image_center, "Dist center");
    displayImage(image_origin, "Dist origin");
    //saveImage(image);

    waitKey();
}