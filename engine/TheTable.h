#pragma once
#include "DrawJob.h"
#include "SceneSubj.h"

class TheTable
{
public:
	//float worldBoxSize[3] = {100,10,100};
	int tableTiles[2] = { 1,1 };
	float tileSize = 100;
	float groundLevel = 0;
	Gabarites worldBox;

	static std::vector<DrawJob*> table_drawJobs;
	static std::vector<unsigned int> table_buffersIds;
	static std::vector<SceneSubj*> tableParts;

public:
	TheTable(float tileSizeXZ, float tileSizeUp, float tileSizeDown, int tilesNx, int tilesNz);
	virtual ~TheTable();
	virtual int init();
	virtual int cleanUp();
};


