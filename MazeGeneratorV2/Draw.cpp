
#include "Draw.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <String>
#include <iostream>
#include <iomanip>
#include <map>

#include "maze.h"
#include "room.h"
#include "markovchain.h"

const int room_x = 100, room_y = 80;
const int offset = 20;
const double font_size = 1;

using namespace cv;

inline void addLine(Mat& img, Point a, Point b) {
	int thickness = 2;
	int lineType = LINE_8;
	
	line(img, a + Point(offset,offset), b + Point(offset, offset), Scalar(255, 255, 255), thickness, lineType);
}

inline void removeLine(Mat& img, Point a, Point b) {
	int thickness = 2;
	int lineType = LINE_8;

	line(img, a + Point(offset, offset), b + Point(offset, offset), Scalar(0, 0, 0), thickness, lineType);
}

const double max_double = 0.01;

inline void drawdouble(Mat& img, Point point, double num) {
	int thickness = 2;
	int lineType = LINE_8;

	double color_scale = ((192.0f * num) / max_double) + 64.0f;

	Point polygon[4];
	polygon[0] = Point(5, 5) + point + Point(offset, offset);
	polygon[1] = Point(5, room_y - 5) + point + Point(offset, offset);
	polygon[2] = Point(room_x - 5, room_y - 5) + point + Point(offset, offset);
	polygon[3] = Point(room_x - 5, 5) + point + Point(offset, offset);

	fillConvexPoly(img, polygon, 4, Scalar(0, 0, color_scale), lineType, 0);

	std::ostringstream ss;
	ss << std::setprecision(2);
	ss << num;

	putText(img, ss.str(), point + Point(offset, offset) + Point(5, 25), FONT_HERSHEY_PLAIN, font_size, Scalar(255, 255, 255), thickness, lineType);
}

Mat drawMaze(Maze& maze, MarkovChain& chain, Vector& dist) {
	int image_width = room_x*maze.getWidth();
	int image_height = room_y*maze.getHeight();

	Mat image = Mat::zeros(image_height + (offset * 2), image_width + (offset * 2), CV_8UC3);

	for (int y = 0; y < maze.getHeight(); ++y) {
		addLine(image, Point(0, y * room_y), Point(image_width, y * room_y));
	}

	addLine(image, Point(0, image_height), Point(image_width, image_height));

	for (int x = 0; x < maze.getWidth(); ++x) {
		addLine(image, Point(x * room_x, 0), Point(x * room_x, image_height));
	}

	addLine(image, Point(image_width, 0), Point(image_width, image_height));
	
	for (int y = 0; y < maze.getHeight(); ++y) {
		if (maze(0, maze.getHeight() - y - 1)(Direction::LEFT))
			removeLine(image, Point(0, room_y * y), Point(0, room_y * (y + 1)));
	}

	for (int x = 0; x < maze.getWidth(); ++x) {
		if (maze(x, maze.getHeight() - 1)(Direction::UP))
			removeLine(image, Point(room_x * x, 0), Point(room_x * (x + 1), 0));
	}

	for (int y = 0; y < maze.getHeight(); ++y) {
		for (int x = 0; x < maze.getWidth(); ++x) {
			if (maze(x, maze.getHeight() - y - 1)(Direction::RIGHT))
				removeLine(image, Point(room_x * (x + 1), room_y * y), Point(room_x * (x + 1), room_y * (y + 1)));

			if (maze(x, maze.getHeight() - y - 1)(Direction::DOWN))
				removeLine(image, Point(room_x * x, room_y * (y + 1)), Point(room_x * (x + 1), room_y * (y + 1)));
		}
	}

	std::map<Room, double> room_map;

	for (int i = 0; i < dist.size(); ++i) {
		auto cords = chain.convertFromIndex(chain.translateFromValidIndex(i));
		room_map[cords.second] += dist(i);
	}

	for (auto it = room_map.begin(); it != room_map.end(); ++it) {
		Room room = it->first;
		double value = it->second;

		drawdouble(image, Point(room_x * room.x, room_y * (maze.getHeight() - 1 - room.y)), value);
	}

	return image;
}

void displayImage(Mat& image, std::string window_name) {
	namedWindow(window_name, WINDOW_AUTOSIZE);
	imshow(window_name, image);
}

void saveImage(Mat& image, std::string filename) {
	imwrite(filename, image);
}