#include "Gabarites.h"
#include "utils.h"

void Gabarites::adjustMidRad(Gabarites* pGB) {
	for (int i = 0; i < 3; i++) {
		pGB->bbRad[i] = (pGB->bbMax[i] - pGB->bbMin[i]) * 0.5f;
		pGB->bbMid[i] = (pGB->bbMax[i] + pGB->bbMin[i]) * 0.5f;
	}
	pGB->boxRad = v3lengthFromTo(pGB->bbMin, pGB->bbMax) / 2;
}
void Gabarites::adjustMinMaxByPoint(Gabarites* pGB, float* newPoint) {
	for (int i = 0; i < 3; i++) {
		if (pGB->bbMin[i] > newPoint[i])
			pGB->bbMin[i] = newPoint[i];
		if (pGB->bbMax[i] < newPoint[i])
			pGB->bbMax[i] = newPoint[i];
	}
}
void Gabarites::toLog(std::string title, Gabarites* pGB) {
	mylog("%s:\n",title.c_str());
	mylog("bbMin: %s\n", v3toStr(pGB->bbMin));
	mylog("bbMax: %s\n", v3toStr(pGB->bbMax));
	mylog("bbMid: %s\n", v3toStr(pGB->bbMid));
	mylog("bbRad: %s\n", v3toStr(pGB->bbRad));
}