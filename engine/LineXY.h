#pragma once
//#include "Vertex01.h"
#include <vector>

class LineXY
{
public:
	float p0[4] = { 0,0,0,0 };
	float p1[4] = { 0,0,0,0 };
	//line equation
	float a_slope = 0; //a
	float b_intercept = 0; //b
	bool isVertical = false;
	float x_vertical = 0;
	bool isHorizontal = false;
	bool isDot = false;
public:
	LineXY(void) {};
	LineXY(float* p00, float* p01) { initLineXY(this, p00, p01); };
	static void calculateLine(LineXY* pLn);
	static void initLineXY(LineXY* pLn, float* p00, float* p01);
	static void initDirXY(LineXY* pLn, float* p01);
	static bool matchingLines(LineXY* pLine0, LineXY* pLine1);
	static bool parallelLines(LineXY* pLine0, LineXY* pLine1);
	static int lineSegmentsIntersectionXY(float* vOut, LineXY* pL1, LineXY* pL2);
	static int linesIntersectionXY(float* vOut, LineXY* pL1, LineXY* pL2);
	static bool isPointIn(float* vOut, LineXY* pL);
	static bool isPointBetween(float* v, float* v1, float* v2);
};
