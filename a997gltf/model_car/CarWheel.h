#pragma once
#include "SceneSubj.h"

class CarWheel : public SceneSubj
{
public:
	float wheelDiameter = 20;
public:
	CarWheel() {};
	CarWheel(CarWheel* pMT0) { memcpy((void*)this, (void*)pMT0, sizeof(CarWheel)); };
	virtual CarWheel* clone() { return new CarWheel(this); };
	virtual int moveSubj() { return moveSubj(this); };
	static int moveSubj(CarWheel* pGS);
};