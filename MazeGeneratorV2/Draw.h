#pragma once

#include <opencv2/core.hpp>
#include "markovchain.h"

class Maze;

cv::Mat drawMaze(Maze& maze, MarkovChain& chain, Vector& dist);
void displayImage(cv::Mat& image, std::string window_name = "Maze");
void saveImage(cv::Mat& image, std::string filename = "imgs/Maze.png");
