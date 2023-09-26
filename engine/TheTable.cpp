#include "TheTable.h"
#include "ModelBuilder.h"
#include "Texture.h"

std::vector<DrawJob*> TheTable::table_drawJobs;
std::vector<unsigned int> TheTable::table_buffersIds;
std::vector<SceneSubj*> TheTable::tableParts;

extern std::string filesRoot;

TheTable::~TheTable() {
	cleanUp();
}
TheTable::TheTable(float tileSizeXZ, float tileSizeUp, float tileSizeDown, int tilesNx, int tilesNz) {
	tileSize = tileSizeXZ;
	tableTiles[0] = tilesNx;
	tableTiles[1] = tilesNz;
	groundLevel = tileSizeDown;
	//define worldBox 
	worldBox.bbMin[1] = 0;
	worldBox.bbMax[1] = tileSizeUp + tileSizeDown;
	worldBox.bbMax[0] = tileSizeXZ * 0.5 * tilesNx;
	worldBox.bbMax[2] = tileSizeXZ * 0.5 * tilesNz;
	worldBox.bbMin[0] = -worldBox.bbMax[0];
	worldBox.bbMin[2] = -worldBox.bbMax[2];
	Gabarites::adjustMidRad(&worldBox);
	init();
}
int TheTable::cleanUp() {
	return 1;
}
int TheTable::init() {
	ModelBuilder* pMB = new ModelBuilder();
	Material mt;
	strcpy_s(mt.shaderType, 32, "phong");
	strcpy_s(mt.materialName, 32, "table");
	mt.dropsShadow = 0;
	//mt.uColor.setRGBA(255, 0, 0);
	mt.uTex0 = Texture::loadTexture(filesRoot+"/dt/mt/wood01.jpg");
	VirtualShape vs;
	v3set(vs.whl, tileSize, groundLevel, tileSize);
	float y0 = groundLevel / 2;
	//table top
	pMB->lockGroup(pMB);
	for (int zN = 0; zN < tableTiles[1]; zN++)
		for (int xN = 0; xN < tableTiles[0]; xN++) {
			SceneSubj* pSS = new SceneSubj();
			tableParts.push_back(pSS);
			pSS->pDrawJobs = &table_drawJobs;
			pSS->pSubjsSet = &tableParts;
			pSS->nInSubjsSet = tableParts.size() - 1;
			pSS->buildModelMatrix();
			pMB->useSubjN(pMB, pSS->nInSubjsSet);
			/*
			if ((zN + xN) % 2 == 0)
				mt.uColor.setRGBA(255, 0, 0);
			else
				mt.uColor.setRGBA(0, 0, 255);

			//mt.uColor.setRGBA(0.5f, 0.5f, 0.5f, 1.0f);
			*/
			pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);

			float z0 = tileSize * zN - worldBox.bbRad[2] + tileSize / 2;
			float x0 = tileSize * xN - worldBox.bbRad[0] + tileSize / 2;

			pMB->lockGroup(pMB);
			pMB->buildBoxFace(pMB, "top", &vs);
			pMB->moveGroupDg(pMB, 0, 0, 0, x0, y0, z0);
			pMB->releaseGroup(pMB);
		}
	TexCoords tc;
	tc.set_GL(&tc, 0,0,10,10,"");
	pMB->groupApplyTexture(pMB, "top", &tc);//, TexCoords * pTC = NULL, TexCoords * pTC2nm = NULL);
	pMB->releaseGroup(pMB);
	pMB->buildDrawJobs(pMB, &tableParts, &table_drawJobs, &table_buffersIds);
	delete pMB;
	return 1;
}
