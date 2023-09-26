#include "Shadows.h"
#include "TheApp.h"
#include "utils.h"

float Shadows::shadowLight = 0.5f;
int Shadows::depthMapTexN = -1;
Texture* Shadows::pDepthMap = NULL;
Camera Shadows::shadowCamera;
float Shadows::sizeUnitPixelsSize = 1;

extern TheApp theApp;

float Shadows::uDepthBias[16]; //z-value shifts depeding on normal
std::vector<SceneSubj*> Shadows::shadowsQueue;



int Shadows::init() {
	if (v3equals(theApp.dirToMainLight, 0)) {
		mylog("ERROR in Shadows::init(): theApp.dirToMainLight not set.\n");
		return -1;
	}

	depthMapTexN = Texture::textures.size();
	Texture* pTex = new Texture();
	Texture::textures.push_back(pTex);
	pTex->size[0] = 2048;
	pTex->size[1] = pTex->size[0];
	pTex->source.assign("depthMap");

	glGenTextures(1, (GLuint*)&pTex->GLid);
	glBindTexture(GL_TEXTURE_2D, pTex->GLid);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		//pTex->size[0], pTex->size[1], 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16F, pTex->size[0], pTex->size[1]);
	checkGLerrors("Depth tex");

	glBindTexture(GL_TEXTURE_2D, 0);
	/////////////////

	Texture::attachRenderBuffer(depthMapTexN, true);
	pDepthMap = Texture::textures.at(depthMapTexN);

	//set up camera
	Camera* pCam = &shadowCamera;
	pCam->viewRangeDg = 0;
	pCam->focusDistance = 100; //temporary
	pCam->targetSize[0] = pTex->size[0];
	pCam->targetSize[1] = pTex->size[1];
	pCam->targetAspectRatio = pCam->targetSize[0] / pCam->targetSize[1];

	float vDir[4];
	for (int i = 0; i < 3; i++) {
		pCam->ownCoords.pos[i] = theApp.dirToMainLight[i] * pCam->focusDistance;
		vDir[i] = -pCam->ownCoords.pos[i];
	}
	//build pCam->ownCoords.eulerDg[]
	pCam->ownCoords.setEulerDg(v3pitchDg(vDir), v3yawDg(vDir), 0);
	//build pCam->ownCoords.rotationMatrix
	mat4x4_from_quat(pCam->ownCoords.rotationMatrix, pCam->ownCoords.getRotationQuat());
	pCam->buildLookAtMatrix(pCam);
	/*
	//set up mViewProjection matrix
	mat4x4 mProjection;
	float nearClip = 0;
	float farClip = worldRadius * 2;
	mat4x4_ortho(mProjection, -worldRadius, worldRadius, -worldRadius, worldRadius, nearClip, farClip);
	mat4x4_mul(shadowCamera.mViewProjection, mProjection, shadowCamera.lookAtMatrix);
	*/

	//calculate uBias (for depth map)
	int totalN = 16;
	float normalStep = 1.0f / (totalN + 1);
	for (int i = 0; i < totalN; i++) {
		float normalZ = normalStep * (i + 1);
		float bias = sqrtf(1.0f - normalZ * normalZ) / normalZ;
		uDepthBias[i] = bias / (float)pDepthMap->size[0] + 0.0005f;// *3.0f;
	}
	return 1;
}


int Shadows::renderDepthMap() {
	Texture::setRenderToTexture(depthMapTexN);
	//Texture* pT = Texture::textures.at(depthMapTexN);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	std::vector<SceneSubj*>* pSubjs = &shadowsQueue;
	//RenderScene
	int subjsN = pSubjs->size();
	for (int subjN = 0; subjN < subjsN; subjN++) {
		SceneSubj* pSS = pSubjs->at(subjN);
		pSS->renderDepthMap(&shadowCamera);
		//checkGLerrors("pGS->renderDepthMap");
	}
	shadowsQueue.clear();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return 1;
}
int Shadows::addToShadowsQueue(std::vector<SceneSubj*>* pSubjs) {
	int subjsN = pSubjs->size();
	int added = 0;
	for (int sN = 0; sN < subjsN; sN++) {
		SceneSubj* pSS = pSubjs->at(sN);
		if (pSS == NULL)
			continue;
		if (pSS->hide > 0)
			continue;
		if (pSS->djTotalN < 1)
			continue;
		if (pSS->dropsShadow == 0)
			continue;
		shadowsQueue.push_back(pSS);
		added++;
	}
	return added;
}
void Shadows::resetShadowsFor(Camera* pCam00, Gabarites* pWorldBox) {
	Gabarites viewBox;
	Gabarites limitBox;
	memcpy(&limitBox, pWorldBox, sizeof(Gabarites));
	std::vector<Line3D*>* pRays = &pCam00->visibleRays;
	int raysN = pRays->size();
	std::vector<Line3D*> adjustedRays;
	memcpy(&viewBox, &pCam00->visibleBox, sizeof(Gabarites));
	/*
	if (pCam00->visibleBox.bbMin[1] > 0)
		memcpy(&viewBox, &pCam00->visibleBox, sizeof(Gabarites));
	else { //if (viewBox.bbMin[1] <= 0) {
		//limit to visible ground area
		viewBox.clear();
		limitBox.bbMax[1] = 10;
		Gabarites::adjustMidRad(&limitBox);
		Line3D ray;
		for (int rayN = 0; rayN < raysN; rayN++) {
			Line3D* pRay0 = pRays->at(rayN);
			memcpy(&ray, pRay0, sizeof(Line3D));
			if (Line3D::fitBox(&ray, &limitBox) == 0)
				continue;
			Gabarites::adjustMinMaxByPoint(&viewBox, ray.p0);
			Gabarites::adjustMinMaxByPoint(&viewBox, ray.p1);
		}
		viewBox.adjustMidRad(&viewBox);
			Gabarites::toLog("Ground pass",&viewBox);
		memcpy(&limitBox, &viewBox, sizeof(Gabarites));
		limitBox.bbMax[1] = pWorldBox->bbMax[1];
		viewBox.adjustMidRad(&limitBox);
		//2nd pass with full height
		viewBox.clear();
		for (int rayN = 0; rayN < raysN; rayN++) {
			Line3D* pRay0 = pRays->at(rayN);
			memcpy(&ray, pRay0, sizeof(Line3D));
			if (Line3D::fitBox(&ray, &limitBox) == 0)
				continue;
			Gabarites::adjustMinMaxByPoint(&viewBox, ray.p0);
			Gabarites::adjustMinMaxByPoint(&viewBox, ray.p1);
			adjustedRays.push_back(new Line3D(ray));
		}
		viewBox.adjustMidRad(&viewBox);
				Gabarites::toLog("Full height pass", &viewBox);
		memcpy(&limitBox, &viewBox, sizeof(Gabarites));
		pRays = &adjustedRays;
	}
	*/

	////////////////////////////////////////
	//set up camera
	Camera* pCam = &shadowCamera;
	pCam->focusDistance = pWorldBox->boxRad*2;
	//reset lookAtPoint according to visibleBox
	v3copy(pCam->lookAtPoint, viewBox.bbMid);
	//reset camera position
	for (int i = 0; i < 3; i++)
		pCam->ownCoords.pos[i] = pCam->lookAtPoint[i] + theApp.dirToMainLight[i] * pCam->focusDistance;
	Camera::buildLookAtMatrix(pCam);

	//set up mViewProjection matrix, adjust view range and clips
	viewBox.clear();
	raysN = pRays->size();
	for (int rayN = 0; rayN < raysN; rayN++) {
		Line3D* pRay = pRays->at(rayN);
		float shadowCamSpacePos[4];
		mat4x4_mul_vec4plus(shadowCamSpacePos, pCam->lookAtMatrix, pRay->p0, 1, true);
		Gabarites::adjustMinMaxByPoint(&viewBox, shadowCamSpacePos);
		mat4x4_mul_vec4plus(shadowCamSpacePos, pCam->lookAtMatrix, pRay->p1, 1, true);
		Gabarites::adjustMinMaxByPoint(&viewBox, shadowCamSpacePos);
	}
	Gabarites::adjustMidRad(&viewBox);

			Gabarites::toLog("from light POV", &viewBox);

	//re-center shadow camera lookAtPoint
	mat4x4 mInverted;
	mat4x4_invert(mInverted, pCam->lookAtMatrix);
	mat4x4_mul_vec4plus(pCam->lookAtPoint, mInverted, viewBox.bbMid, 1, true);
	//reset camera position
	for (int i = 0; i < 3; i++)
		pCam->ownCoords.pos[i] = pCam->lookAtPoint[i] + theApp.dirToMainLight[i] * pCam->focusDistance;
	Camera::buildLookAtMatrix(pCam);

	//set up mViewProjection matrix, adjust view range and clips
	viewBox.clear();
	for (int rayN = 0; rayN < raysN; rayN++) {
		Line3D* pRay = pRays->at(rayN);
		float shadowCamSpacePos[4];
		mat4x4_mul_vec4plus(shadowCamSpacePos, pCam->lookAtMatrix, pRay->p0, 1, true);
		Gabarites::adjustMinMaxByPoint(&viewBox, shadowCamSpacePos);
		mat4x4_mul_vec4plus(shadowCamSpacePos, pCam->lookAtMatrix, pRay->p1, 1, true);
		Gabarites::adjustMinMaxByPoint(&viewBox, shadowCamSpacePos);
	}
	Gabarites::adjustMidRad(&viewBox);

		Gabarites::toLog("after re-center", &viewBox);

	pCam->stageSize[0] = std::max((viewBox.bbMax[0] - viewBox.bbMin[0]), (viewBox.bbMax[1] - viewBox.bbMin[1]));
			pCam->stageSize[0] = pCam->stageSize[0]*0.2 + pCam00->stageSize[0]*0.8;
	pCam->stageSize[1] = pCam->stageSize[0];
	Camera::setNearAndFarClips(pCam, pWorldBox);
					pCam->farClip = -viewBox.bbMin[2];
	Camera::buildViewProjectionWithClips(pCam);

	sizeUnitPixelsSize = pDepthMap->size[0] / pCam->stageSize[0];

}

