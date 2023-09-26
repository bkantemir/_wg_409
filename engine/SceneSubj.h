#pragma once
#include "Coords.h"
#include "Camera.h"
#include "MaterialAdjust.h"
#include <string>
#include <vector>
#include "DrawJob.h"
#include "Gabarites.h"
#include "Shader.h"

class SceneSubj
{
public:
    std::vector<DrawJob*>* pDrawJobs = NULL; //which vector/set this subj belongs to
    std::vector<SceneSubj*>* pSubjsSet = NULL; //which vector/set this subj belongs to
    int nInSubjsSet = 0; //subj's number in pSubjsSet
    int rootN = 0; //model's root N
    int d2parent = 0; //shift to parent object
    int d2headTo = 0; //shift to headTo object
    SceneSubj* pStickTo = NULL;
    int totalElements = 0; //elements N in the model
    int totalNativeElements = 0; //elements N in the model when initially loaded
    char source[256] = "";
    char className[32] = "";
    Coords absCoords;
    mat4x4 absModelMatrixUnscaled = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
    mat4x4 absModelMatrix = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
    mat4x4 ownModelMatrixUnscaled = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
    uint32_t absCoordsUpdateFrameN = -1;

    char name[64] = "";
    Coords ownCoords;
    Coords ownSpeed;
    float scale[3] = { 1,1,1 };
    int djStartN = 0; //first DJ N in DJs array (DrawJob::drawJobs)
    int djTotalN = 0; //number of DJs
    Material mt0;
    Material mt0Layer2; //second layer
    int mt0isSet = 0;
    int dropsShadow = 1;
    int hide = 0;//1-hide, 2-hide forever (delete)

    char fileOnDeploy[256] = "";
    std::vector<MyColor*>* pCustomColors = NULL;
    std::vector<MaterialAdjust*>* pCustomMaterials = NULL;

    Gabarites gabaritesOnLoad;
    Gabarites gabaritesWorld;
    Gabarites gabaritesOnScreenGL;
    int isOnScreen = 0; //1-completely on-screen, 0 - partially, -1 - off-screen

    int renderOrder = -1; //-1-unknown, 0-render 1st (no alpha or b/w), 1-render last (has alpa), 2-render last (disordered mixed alpha)
public:
    SceneSubj();
    SceneSubj(SceneSubj* pSS0) { memcpy((void*)this, (void*)pSS0, sizeof(SceneSubj)); };
    virtual ~SceneSubj();
    virtual SceneSubj* clone() { return new SceneSubj(this); };
    virtual void buildModelMatrix() { buildModelMatrixStandard(this); };
    static void buildModelMatrixStandard(SceneSubj* pGS);
    virtual int moveSubj() { return applySpeeds(this); };
    static int applySpeeds(SceneSubj* pGS);
    virtual int render(Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter) {
        return renderStandard(this, pCam, dirToMainLight, dirToTranslucent, renderFilter);
    };
    static int renderStandard(SceneSubj* pGS, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter);

    virtual int renderDepthMap(Camera* pCam) { return renderDepthMapStandard(this, pCam); };
    static int renderDepthMapStandard(SceneSubj* pGS, Camera* pCam);
    virtual int scaleMe(float k) { return scaleStandard(this, k); };
    static int scaleStandard(SceneSubj* pGS, float k);
    virtual int onDeploy() { return onDeployStandatd(this); };
    static int onDeployStandatd(SceneSubj* pSS);
    virtual int processSubj() { return processSubjStandart(this); };
    static int processSubjStandart(SceneSubj* pGS);
    static int setRoot(SceneSubj* pGS);
    static int getScreenPosition(SceneSubj* pSS, mat4x4 mViewProjection);// , Camera* pCam, float* dirToMainLight);

    static int addToRenderQueue(std::vector<SceneSubj*>* pQueueOpaque, std::vector<SceneSubj*>* pQueueTransparent, std::vector<SceneSubj*>* pSubjs);
    static int sortRenderQueue(std::vector<SceneSubj*>* pQueue, int direction); //0-closer 1st, 1-farther 1st

    static int customizeMaterial(Material** ppMt, Material** ppMt2, Material** ppAltMt, Material** ppAltMt2, SceneSubj* pSS);
    static int executeDJstandard(DrawJob* pDJ, float* uMVP, float* uMV3x3, float* uMM, float* uMVP4dm,
        float* uVectorToLight, float* uVectorToTranslucent, float* uCameraPosition, float line1pixSize, Material* pMt);
    static bool lineWidthIsImportant(int primitiveType);
    static int executeDJcommon(DrawJob* pDJ, Shader* pShader,
        float* uVectorToLight, float* uVectorToTranslucent, float* uCameraPosition, Material* pMt);
    static void fillBBox(Gabarites* pGB, SceneSubj* pSS0, mat4x4 mMVP);
};


