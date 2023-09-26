#pragma once
#include <vector>
#include "LineXY.h"
#include "utils.h"

class Gabarites
{
public:
	//bounding box
	float bbMin[3] = { 1000000,1000000,1000000 };
	float bbMax[3] = { -1000000,-1000000,-1000000 };

	float bbMid[4];
	float bbRad[3];

	float sizeDirXY[3][4];
	float sizeXY[3];
	//chord
	float chordR;
	LineXY chord;

	float boxRad = 0;
public:
	void clear() { v3setAll(this->bbMin, 1000000); v3setAll(this->bbMax, -1000000);	};
	static void adjustMidRad(Gabarites* pGB);
	static void adjustMinMaxByPoint(Gabarites* pGB, float* newPoint);

	static void toLog(std::string title,Gabarites* pGB);

};

