
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <vector>
#include "Draw.h"
#include "Maze.h"
#include "Room.h"

using namespace cv;

int main()
{
    Maze maze(10, 10);

    MarkovChain chain(maze);

    chain.validateMarkovChain();
    Vector dist = chain.getOccupancyDistribution();
    Mat image = drawMaze(maze, chain, dist);
    displayImage(image, "Dist");
    
    //Matrix P = chain.getNStepMatrix(300);
    ////Matrix P2 = chain.getNStepMatrix(301);

    //Vector ones(P.rows());
    //ones.setOnes();
    //ones = P * (ones / P.rows());


    //Room center(5, 5, maze);
    //Vector center_dist = P * setupDistribution(chain, maze, center);

    //Room origin(0, 0, maze);
    //Vector origin_dist = P * setupDistribution(chain, maze, origin);

    //Mat image_ones = drawMaze(maze, chain, ones);
    //Mat image_center = drawMaze(maze, chain, center_dist);
    //Mat image_origin = drawMaze(maze, chain, origin_dist);
    //displayImage(image_ones, "Dist ones");
    //displayImage(image_center, "Dist center");
    //displayImage(image_origin, "Dist origin");
    //saveImage(image);

    waitKey();
}