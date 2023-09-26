#include "LineXY.h"
#include "utils.h"

void LineXY::initDirXY(LineXY* pLn, float* p01) {
	float p00[3] = { 0,0 };
	initLineXY(pLn, p00, p01);
}
void LineXY::initLineXY(LineXY* pLn, float* p00, float* p01) {
	v2copy(pLn->p0, p00);
	v2copy(pLn->p1, p01);
	calculateLine(pLn);
}
void LineXY::calculateLine(LineXY* pLn) {
	if (v2match(pLn->p0, pLn->p1))
		pLn->isDot = true;
	else {
		pLn->isDot = false;
		//find line equation
		if (pLn->p0[0] == pLn->p1[0]) {
			pLn->isVertical = true;
			pLn->x_vertical = pLn->p0[0];
		}
		else if (pLn->p0[1] == pLn->p1[1]) {
			pLn->isHorizontal = true;
			pLn->a_slope = 0;
			pLn->b_intercept = pLn->p0[1];
		}
		else {
			pLn->a_slope = (pLn->p1[1] - pLn->p0[1]) / (pLn->p1[0] - pLn->p0[0]);
			pLn->b_intercept = pLn->p0[1] - pLn->a_slope * pLn->p0[0];
		}
	}
}
bool LineXY::matchingLines(LineXY* pLine0, LineXY* pLine1) {
	if (!parallelLines(pLine0, pLine1))
		return false;
	if (pLine0->b_intercept != pLine1->b_intercept)
		return false;
	if (pLine0->x_vertical != pLine1->x_vertical)
		return false;
	return true;
}
bool LineXY::parallelLines(LineXY* pLine0, LineXY* pLine1) {
	if (pLine0->isVertical != pLine1->isVertical)
		return false;
	if (pLine0->a_slope != pLine1->a_slope)
		return false;
	return true;
}
int LineXY::linesIntersectionXY(float* vOut, LineXY* pL1, LineXY* pL2) {
	//returns 0 if no intersection
	if (parallelLines(pL1, pL2))
		return 0;
	//find lines intersection, assuming lines are not parallel
	float x, y;
	if (pL1->isVertical) {
		x = pL1->p0[0];
		y = pL2->a_slope * x + pL2->b_intercept;
	}
	else { //pL1 not vertical
		if (pL2->isVertical) {
			x = pL2->p0[0];
			y = pL1->a_slope * x + pL1->b_intercept;
		}
		else { //both lines are "normal"
			x = (pL2->b_intercept - pL1->b_intercept) / (pL1->a_slope - pL2->a_slope);
			y = pL1->a_slope * x + pL1->b_intercept;
		}
	}
	vOut[0] = x;
	vOut[1] = y;
	return 1;
}
int LineXY::lineSegmentsIntersectionXY(float* vOut, LineXY* pL1, LineXY* pL2) {
	//returns 0 if no intersection
	if(linesIntersectionXY(vOut,pL1, pL2) == 0)
		return 0; //lines not crossing
	if (!isPointBetween(vOut, pL1->p0, pL1->p1))
		return 0; //intersection is out of range
	return 1;
}
bool LineXY::isPointIn(float* v, LineXY* pL1) {
	if (v[1] != (pL1->a_slope * v[0] + pL1->b_intercept))
		return false; //not on line
	if (isPointBetween(v, pL1->p0, pL1->p1))
		return true;
	return false;
}

bool LineXY::isPointBetween(float* v, float* v1, float* v2) {
	for(int i=0;i<2;i++)
		if(((v[i] - v1[i]) * (v[i] - v2[i])) > 0)
		return false;
	return true;
}

