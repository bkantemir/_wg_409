#pragma once
#include "Camera.h"
#include "SceneSubj.h"
#include "Texture.h"

class Shadows
{
public:
	static float shadowLight;

	static int depthMapTexN;
	static Texture* pDepthMap;

	static Camera shadowCamera;
	static float sizeUnitPixelsSize;

	static float uDepthBias[16]; //z-value shifts depeding on normal
	static std::vector<SceneSubj*> shadowsQueue;
public:
	static int init();
	static int addToShadowsQueue(std::vector<SceneSubj*>* pGSubjs);
	static int renderDepthMap();
	static void resetShadowsFor(Camera* pCam, Gabarites* pWorldBox);

};


