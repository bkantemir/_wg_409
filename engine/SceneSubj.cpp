#include "SceneSubj.h"
#include "platform.h"
#include "utils.h"
#include "TheApp.h"
#include "DrawJob.h"
#include "Shadows.h"
#include "ModelLoaderCmd.h"
#include <algorithm>


extern TheApp theApp;
extern float degrees2radians;

SceneSubj::SceneSubj() {
}
SceneSubj::~SceneSubj() {
    if (pCustomColors != NULL) {
        int totalN = pCustomColors->size();
        for (int i = 0; i < totalN; i++)
            delete pCustomColors->at(i);
        pCustomColors->clear();
        delete pCustomColors;
        pCustomColors = NULL;
    }
    if (pCustomMaterials != NULL) {
        int totalN = pCustomMaterials->size();
        for (int i = 0; i < totalN; i++)
            delete pCustomMaterials->at(i);
        pCustomMaterials->clear();
        delete pCustomMaterials;
        pCustomMaterials = NULL;
    }
}
void SceneSubj::buildModelMatrixStandard(SceneSubj* pSS) {
    mat4x4_translate(pSS->ownModelMatrixUnscaled, pSS->ownCoords.pos[0], pSS->ownCoords.pos[1], pSS->ownCoords.pos[2]);
    //rotation order: Z-X-Y
    mat4x4_mul(pSS->ownModelMatrixUnscaled, pSS->ownModelMatrixUnscaled, pSS->ownCoords.rotationMatrix);

    if (pSS->d2parent != 0) {
        SceneSubj* pParent = pSS->pSubjsSet->at(pSS->nInSubjsSet - pSS->d2parent);
        mat4x4_mul(pSS->absModelMatrixUnscaled, pParent->absModelMatrixUnscaled, pSS->ownModelMatrixUnscaled);
    }
    else if (pSS->pStickTo != NULL)
        mat4x4_mul(pSS->absModelMatrixUnscaled, pSS->pStickTo->absModelMatrixUnscaled, pSS->ownModelMatrixUnscaled);
    else
        memcpy(pSS->absModelMatrixUnscaled, pSS->ownModelMatrixUnscaled, sizeof(mat4x4));

    if (v3equals(pSS->scale, 1))
        memcpy(pSS->absModelMatrix, pSS->absModelMatrixUnscaled, sizeof(mat4x4));
    else
        mat4x4_scale_aniso(pSS->absModelMatrix, pSS->absModelMatrixUnscaled, pSS->scale[0], pSS->scale[1], pSS->scale[2]);

    //update absCoords
    if (pSS->d2parent == 0)
        memcpy(&pSS->absCoords, &pSS->ownCoords, sizeof(Coords));
    else {
        Coords::getPositionFromMatrix(pSS->absCoords.pos, pSS->absModelMatrixUnscaled);
    }

    pSS->absCoordsUpdateFrameN = theApp.frameN;
    /*
    if (pSS->nInSubjsSet == 3) {
        //pSS->absCoords.rotationMatrix2quat();
        quat_from_mat4x4(pSS->absCoords.getRotationQuat(), pSS->absModelMatrixUnscaled);
        //quat_norm(pSS->absCoords.getRotationQuat(), pSS->absCoords.getRotationQuat());
        pSS->absCoords.quaternion2eulerDg();

        mylog("mat0=%d/%d/%d/%d\n", (int)(pSS->absModelMatrixUnscaled[0][0] * 100), (int)(pSS->absModelMatrixUnscaled[0][1] * 100), (int)(pSS->absModelMatrixUnscaled[0][2] * 100), (int)(pSS->absModelMatrixUnscaled[0][3] * 100));
        mylog("mat1=%d/%d/%d/%d\n", (int)(pSS->absModelMatrixUnscaled[1][0] * 100), (int)(pSS->absModelMatrixUnscaled[1][1] * 100), (int)(pSS->absModelMatrixUnscaled[1][2] * 100), (int)(pSS->absModelMatrixUnscaled[1][3] * 100));
        mylog("mat2=%d/%d/%d/%d\n", (int)(pSS->absModelMatrixUnscaled[2][0] * 100), (int)(pSS->absModelMatrixUnscaled[2][1] * 100), (int)(pSS->absModelMatrixUnscaled[2][2] * 100), (int)(pSS->absModelMatrixUnscaled[2][3] * 100));
        mylog("mat3=%d/%d/%d/%d\n", (int)(pSS->absModelMatrixUnscaled[3][0] * 100), (int)(pSS->absModelMatrixUnscaled[3][1] * 100), (int)(pSS->absModelMatrixUnscaled[3][2] * 100), (int)(pSS->absModelMatrixUnscaled[3][3] * 100));


        mylog("quat=%d/%d/%d/%d - ", (int)(pSS->absCoords.getRotationQuat(0) * 100), (int)(pSS->absCoords.getRotationQuat(1) * 100), (int)(pSS->absCoords.getRotationQuat(2) * 100), (int)(pSS->absCoords.getRotationQuat(3) * 100));
        mylog("Euler=%d/%d/%d frame=%d onScreen=%d\n", (int)pSS->absCoords.getEulerDg(0), (int)pSS->absCoords.getEulerDg(1), (int)pSS->absCoords.getEulerDg(2), (int)pSS->absCoordsUpdateFrameN, pSS->isOnScreen);
        int a = 0;
    }
    */
}
int SceneSubj::applySpeeds(SceneSubj* pSS) {
    if (pSS->ownSpeed.noRotation())
        return 0;
    quat q2;
    quat_mul(q2, pSS->ownSpeed.getRotationQuat(), pSS->ownCoords.getRotationQuat());
    pSS->ownCoords.setQuaternion(q2);
    //mat4x4_from_quat(pSS->ownCoords.rotationMatrix, q2);
    return 1;
}


int SceneSubj::renderStandard(SceneSubj* pSS, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter) {
    if (renderFilter == pSS->renderOrder)
        renderFilter = -1; //render all

    //build mMVP4dm (shadow MVP) matrix for given subject
    mat4x4 mMVP4dm;
    mat4x4_mul(mMVP4dm, Shadows::shadowCamera.mViewProjection, pSS->absModelMatrix);

    //build MVP matrix for given subject
    mat4x4 mMVP;
    mat4x4_mul(mMVP, pCam->mViewProjection, pSS->absModelMatrix);
    //build Model-View (rotation) matrix for normals
    mat4x4 mMV4x4;
    mat4x4_mul(mMV4x4, pCam->lookAtMatrix, pSS->absModelMatrixUnscaled);
    //convert to 3x3 matrix
    float mMV3x3[3][3];
    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 3; x++)
            mMV3x3[y][x] = mMV4x4[y][x];

    //subj's distance from camera
    float cameraSpacePos[4];
    mat4x4_mul_vec4plus(cameraSpacePos, pCam->lookAtMatrix, pSS->absCoords.pos, 1);
    float zDistance = abs(cameraSpacePos[2]);
    float viewRangeDg = pCam->viewRangeDg;
    if (viewRangeDg == 0)
        viewRangeDg = 30;
    float cotangentA = 1.0f / tanf(degrees2radians * viewRangeDg / 2.0f);
    float halfScreenVertSizeInUnits = zDistance / cotangentA;
    float sizeUnitPixelsSize = (float)theApp.mainCamera.targetSize[1] / 2.0f / halfScreenVertSizeInUnits;
    sizeUnitPixelsSize *= pSS->scale[0];
    //render subject
    if (pSS->pDrawJobs == NULL)
        pSS->pDrawJobs = &DrawJob::drawJobs_default;

    for (int i0 = 0; i0 < pSS->djTotalN; i0++) {
        int i = i0;
        if (renderFilter > 0) //render transparent only, most likely - GLTF reversed order
            i = pSS->djTotalN - 1 - i0;

        DrawJob* pDJ = pSS->pDrawJobs->at(pSS->djStartN + i);

        Material* pMt = &pDJ->mt;

        if (renderFilter >= 0)
            if (renderFilter != pMt->uAlphaBlending > 0)
                continue;

        Material* pMt2 = &pDJ->mtLayer2;
        if (i == 0) {
            Material mt;
            memcpy(&mt, &pDJ->mt, sizeof(Material));
            pMt = &mt;
            //2-nd layer
            Material mt2;
            memcpy(&mt2, &pDJ->mtLayer2, sizeof(Material));
            pMt2 = &mt2;
        }
        Material* pAltMt = NULL;
        Material* pAltMt2 = NULL;
        if (i == 0 && pSS->mt0isSet > 0) { //already customized
            pMt = &pSS->mt0;
            pMt2 = &pSS->mt0Layer2;
        }
        else { //customize?
            customizeMaterial(&pMt, &pMt2, &pAltMt, &pAltMt2, pSS);
            if (i == 0) {
                memcpy(&pSS->mt0, pMt, sizeof(Material));
                memcpy(&pSS->mt0Layer2, pMt2, sizeof(Material));
                pSS->mt0isSet = 1;
            }
        }
        executeDJstandard(pDJ, (float*)mMVP, *mMV3x3, (float*)pSS->absModelMatrix, (float*)mMVP4dm, dirToMainLight, dirToTranslucent,
            pCam->ownCoords.pos, sizeUnitPixelsSize, pMt);

        //have 2-nd layer ?
        if (pMt2 != NULL)
            if (pMt2->shaderN >= 0) {
                executeDJstandard(pDJ, (float*)mMVP, *mMV3x3, (float*)pSS->absModelMatrix, (float*)mMVP4dm, dirToMainLight, dirToTranslucent,
                    pCam->ownCoords.pos, sizeUnitPixelsSize, pMt2);
            }

        if (pAltMt != NULL) {
            delete pAltMt;
            pAltMt = NULL;
        }
        if (pAltMt2 != NULL) {
            delete pAltMt2;
            pAltMt2 = NULL;
        }
    }
    return 1;
}


int SceneSubj::scaleStandard(SceneSubj* pSS, float k) {
    for (int i = 0; i < 3; i++)
        pSS->scale[i] *= k;
    int subjsN = pSS->pSubjsSet->size();
    for (int sN = pSS->nInSubjsSet + 1; sN < subjsN; sN++) {
        SceneSubj* pSS2 = pSS->pSubjsSet->at(sN);
        if (pSS2 == NULL)
            continue;
        if (pSS2->nInSubjsSet - pSS2->d2parent != pSS->nInSubjsSet)
            continue;
        for (int i = 0; i < 3; i++)
            pSS2->ownCoords.pos[i] *= k;
        pSS2->scaleMe(k);
    }
    return 1;
}

int SceneSubj::setRoot(SceneSubj* pSS00) {
    int rootN = pSS00->nInSubjsSet;
    std::vector<SceneSubj*> subjs2check;
    subjs2check.push_back(pSS00);
    int totalN = pSS00->pSubjsSet->size();
    while (subjs2check.size() > 0) {
        SceneSubj* pParent = subjs2check.back();
        subjs2check.pop_back();
        pParent->rootN = rootN;
        int parentN = pParent->nInSubjsSet;
        for (int ssN = parentN + 1; ssN < totalN; ssN++) {
            SceneSubj* pChild = pParent->pSubjsSet->at(ssN);
            if (pChild->nInSubjsSet - pChild->d2parent != parentN)
                continue;
            subjs2check.push_back(pChild);
        }
    }
    return 1;
}
int SceneSubj::processSubjStandart(SceneSubj* pSS) {
    //check root
    if (pSS->d2parent == 0)
        pSS->rootN = pSS->nInSubjsSet;
    else {
        int parentN = pSS->nInSubjsSet - pSS->d2parent;
        SceneSubj* pParent = pSS->pSubjsSet->at(parentN);
        //inherit parent
        pSS->rootN = pParent->rootN;
        pSS->hide = pParent->hide;
    }

    pSS->moveSubj();
    if (pSS->absCoordsUpdateFrameN != theApp.frameN)
        pSS->buildModelMatrix();
    return 1;
}

int SceneSubj::renderDepthMapStandard(SceneSubj* pSS, Camera* pCam) {
    if (pSS->pDrawJobs == NULL)
        pSS->pDrawJobs = &DrawJob::drawJobs_default;

    if (pSS->dropsShadow < 1)
        return 0;
    mat4x4 mMVP;
    mat4x4_mul(mMVP, pCam->mViewProjection, pSS->absModelMatrix);
    mat4x4 mMV4x4;
    mat4x4_mul(mMV4x4, pCam->lookAtMatrix, pSS->absModelMatrixUnscaled);
    //convert to 3x3 matrix
    float mMV3x3[3][3];
    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 3; x++)
            mMV3x3[y][x] = mMV4x4[y][x];
    //render subject
    for (int i = 0; i < pSS->djTotalN; i++) {
        DrawJob* pDJ = pSS->pDrawJobs->at(pSS->djStartN + i);
        if (pDJ->mt.dropsShadow < 1)
            continue;
        Material* pMT = &pDJ->mtShadow;

        executeDJstandard(pDJ, (float*)mMVP, *mMV3x3, NULL, NULL, NULL, NULL, NULL,
            Shadows::sizeUnitPixelsSize * pSS->scale[0], pMT);
    }

    /*
    float vIn[4] = { 0,0,-1,0 };
    float vOut[4];
    mat4x4_mul_vec4plus(vOut, mMVP, vIn, 1);
    */
    return 1;
}

int SceneSubj::getScreenPosition(SceneSubj* pSS, mat4x4 mViewProjection) { //, Camera* pCam, float* dirToMainLight) {
    //build MVP matrix for given subject
    mat4x4 mMVP;
    mat4x4_mul(mMVP, mViewProjection, pSS->absModelMatrix);

    fillBBox(&pSS->gabaritesOnScreenGL, pSS, mMVP);
    /*
    //calculate mid point position
    v3toGL(pSS->gabaritesOnScreenGL.bbMid, mMVP, pSS->gabaritesOnLoad.bbMid, 1);
    //build size vectors
    float sizeVector[3][4];
    float sizeDirXY[3][4];
    float sizeXY[3];
    for (int axisN = 0; axisN < 3; axisN++) {
        if (pSS->gabaritesOnLoad.bbRad[axisN] == 0) {
            v3setAll(sizeVector[axisN], 0);
            v3setAll(sizeDirXY[axisN], 0);
            sizeXY[axisN] = 0;
            continue;
        }
        float vIn[4];
        v3copy(vIn, pSS->gabaritesOnLoad.bbMid);
        vIn[axisN] += pSS->gabaritesOnLoad.bbRad[axisN];
        float vOut[4];
        v3toGL(vOut, mMVP, vIn, 1);

        v3fromTo(sizeVector[axisN], pSS->gabaritesOnScreenGL.bbMid, vOut);
        v3normXY(sizeDirXY[axisN], sizeVector[axisN]);
        sizeXY[axisN] = v3lengthXY(sizeVector[axisN]);
    }
    //calculate on-screen bounding box
    v3setAll(pSS->gabaritesOnScreenGL.bbMin, 1000000);
    v3setAll(pSS->gabaritesOnScreenGL.bbMax, -1000000);
    for (int z = -1; z <= 1; z += 2)
        for (int y = -1; y <= 1; y += 2)
            for (int x = -1; x <= 1; x += 2)
                for (int i = 0; i < 3; i++) {
                    float xx = pSS->gabaritesOnScreenGL.bbMid[i] + sizeVector[2][i] * z + sizeVector[1][i] * y + sizeVector[0][i] * x;
                    if (pSS->gabaritesOnScreenGL.bbMin[i] > xx)
                        pSS->gabaritesOnScreenGL.bbMin[i] = xx;
                    if (pSS->gabaritesOnScreenGL.bbMax[i] < xx)
                        pSS->gabaritesOnScreenGL.bbMax[i] = xx;
                }
                */
    Gabarites* pGB = &pSS->gabaritesOnScreenGL;
    //check if off-screen
    int score = 0;
    for (int i = 0; i < 3; i++) {
        if (pSS->gabaritesOnScreenGL.bbMin[i] > 1) {
            score = -1;
            break;
        }
        if (pSS->gabaritesOnScreenGL.bbMax[i] < -1) {
            score = -1;
            break;
        }
        if (pSS->gabaritesOnScreenGL.bbMin[i] > -1)
            score++;
        if (pSS->gabaritesOnScreenGL.bbMax[i] < 1)
            score++;
    }
    if (score < 0)
        pSS->isOnScreen = -1; //off-screen
    else if (score == 6)
        pSS->isOnScreen = 1; //on-screen
    else
        pSS->isOnScreen = 0; //partially

    pSS->isOnScreen = 0;

    if (pSS->isOnScreen < 0)
        return 0;

    //select biggest axis
    int biggestAxisN = 0;
    float biggestSize = 0;
    for (int axisN = 0; axisN < 3; axisN++) {
        if (biggestSize >= pGB->sizeXY[axisN])
            continue;
        biggestSize = pGB->sizeXY[axisN];
        biggestAxisN = axisN;
    }
    float* pDirChord = pGB->sizeDirXY[biggestAxisN];
    //perpendicular
    float pDirPerp[2];
    pDirPerp[0] = pDirChord[1];
    pDirPerp[1] = -pDirChord[0];
    float chordH = 0; //half-length
    float chordR = 0; //rad
    for (int i = 0; i < 3; i++) {
        if (i == biggestAxisN) {
            chordH += pGB->sizeXY[i];
            continue;
        }
        if (pGB->sizeXY[i] == 0)
            continue;
        chordH = chordH + abs(v2dotProductNorm(pDirChord, pGB->sizeDirXY[i])) * pGB->sizeXY[i];
        chordR = chordR + abs(v2dotProductNorm(pDirPerp, pGB->sizeDirXY[i])) * pGB->sizeXY[i];
    }
    pSS->gabaritesOnScreenGL.chordR = chordR;
    chordH -= chordR;
    if (chordH < 1.0f) //dot
        pSS->gabaritesOnScreenGL.chord.initLineXY(&pSS->gabaritesOnScreenGL.chord,pSS->gabaritesOnScreenGL.bbMid, pSS->gabaritesOnScreenGL.bbMid);
    else {
        float dot0[2];
        float dot1[2];
        for (int i = 0; i < 2; i++) {
            float d = pDirChord[i] * chordH;
            dot0[i] = pSS->gabaritesOnScreenGL.bbMid[i] + d;
            dot1[i] = pSS->gabaritesOnScreenGL.bbMid[i] - d;
        }
        pSS->gabaritesOnScreenGL.chord.initLineXY(&pSS->gabaritesOnScreenGL.chord,dot0, dot1);
    }
    return 1;
}
int SceneSubj::onDeployStandatd(SceneSubj* pSS) {
    
    if (strlen(pSS->fileOnDeploy) == 0)
        return 0;
    std::string fileOnDeployStr(pSS->fileOnDeploy);
    ModelLoaderCmd* pML = new ModelLoaderCmd(pSS, fileOnDeployStr);
    pML->processSource(pML);
    delete pML;
    
    return 1;
}
int SceneSubj::addToRenderQueue(std::vector<SceneSubj*>* pQueueOpaque, std::vector<SceneSubj*>* pQueueTransparent, std::vector<SceneSubj*>* pSubjs) {
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
        if (pSS->isOnScreen < 0)
            continue;
        //check render order/priority
        if (pSS->renderOrder < 0) {
            int lastOpaqueN = -1;
            int lastAlphaN = -1;
            for (int djN = 0; djN < pSS->djTotalN; djN++) {
                DrawJob* pDJ = pSS->pDrawJobs->at(pSS->djStartN + djN);
                Material* pMt = &pDJ->mt;
                if (pMt->uAlphaBlending > 0) //transparent
                    lastAlphaN = djN;
                else //opaque
                    lastOpaqueN = djN;
            }
            if (lastAlphaN < 0)//all DJs opaque
                pSS->renderOrder = 0;
            else if (lastOpaqueN < 0)
                pSS->renderOrder = 1; //all w/alpha
            else
                pSS->renderOrder = 2; //some DJs w/alpha, some - opaque, mixed
        }

        if (pQueueTransparent == NULL)
            pQueueOpaque->push_back(pSS);
        else if (pSS->renderOrder == 0)
            pQueueOpaque->push_back(pSS);
        else if (pSS->renderOrder == 1)
            pQueueTransparent->push_back(pSS);
        else { //mixed
            pQueueOpaque->push_back(pSS);
            pQueueTransparent->push_back(pSS);
        }
        added++;
    }
    return added;
}
int SceneSubj::sortRenderQueue(std::vector<SceneSubj*>* pQueue, int direction) {
    //direction: 0-closer 1st, 1-farther 1st
    if (direction == 0) //0-closer 1st
        std::sort(pQueue->begin(), pQueue->end(),
            [](SceneSubj* pSS0, SceneSubj* pSS1) {
                //if < - go 1st
                return pSS0->gabaritesOnScreenGL.bbMin[2] < pSS1->gabaritesOnScreenGL.bbMin[2];
            });
    else //1-farther 1st
        std::sort(pQueue->begin(), pQueue->end(),
            [](SceneSubj* pSS0, SceneSubj* pSS1) {
                //if > - go 1st
                return pSS0->gabaritesOnScreenGL.bbMin[2] > pSS1->gabaritesOnScreenGL.bbMin[2];
            });
    return 1;
}

int SceneSubj::customizeMaterial(Material** ppMt, Material** ppMt2, Material** ppAltMt, Material** ppAltMt2, SceneSubj* pSS) {
    Material* pMt = *ppMt;
    Material* pMt2 = *ppMt2;
    Material* pAltMt = *ppAltMt;
    Material* pAltMt2 = *ppAltMt2;

    SceneSubj* pRoot = pSS->pSubjsSet->at(pSS->rootN);
    if (pRoot->pCustomMaterials != NULL) {
        int customMaterialsN = pRoot->pCustomMaterials->size();
        if (strlen(pMt->materialName) != 0) {
            for (int oN = 0; oN <= customMaterialsN; oN++) {
                if (oN == customMaterialsN)
                    break;
                MaterialAdjust* pMA = pRoot->pCustomMaterials->at(oN); //custom color option
                if (strcmp(pMA->materialName, pMt->materialName) == 0) {
                    pAltMt = new Material(pMt);
                    pMt = pAltMt;
                    pMA->adjust(pMt, pMA);
                    Shader::pickShaderNumber(pMt);
                    //2-nd layer
                    if (strlen(pMt->layer2as) == 0)
                        //pMt2 = NULL;
                        pMt2->shaderN = -1;
                    else if (strcmp(pMt->layer2as, pMt2->materialName) != 0) {
                        pAltMt2 = new Material(pMt);
                        pMt2 = pAltMt2;
                        pMt2->shaderN = -1;
                        strcpy_s(pMt2->materialName, 32, pMt->layer2as);
                        //look in
                        std::vector<MaterialAdjust*>* pMAlist = &MaterialAdjust::materialAdjustsList;
                        customMaterialsN = pMAlist->size();
                        for (int oN = 0; oN < customMaterialsN; oN++) {
                            MaterialAdjust* pMA = pMAlist->at(oN); //custom color option
                            if (strcmp(pMA->materialName, pMt2->materialName) == 0) {
                                pMA->adjust(pMt2, pMA);
                                Shader::pickShaderNumber(pMt2);
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    if (pRoot->pCustomColors != NULL) {
        int customColorsN = pRoot->pCustomColors->size();
        //customize colors?
        MyColor* pColors[3];
        pColors[0] = &pMt->uColor;
        pColors[1] = &pMt->uColor1;
        pColors[2] = &pMt->uColor2;
        for (int cN = 0; cN < 3; cN++) {
            MyColor* pCL = pColors[cN];
            if (strlen(pCL->colorName) == 0)
                continue;
            for (int oN = 0; oN <= customColorsN; oN++) {
                if (oN == customColorsN)
                    break;
                MyColor* pCC = pRoot->pCustomColors->at(oN); //custom color option
                if (strcmp(pCC->colorName, pCL->colorName) == 0) {
                    memcpy(pCL, pCC, sizeof(MyColor));
                    break;
                }
            }
        }
    }
    *ppMt = pMt;
    *ppMt2 = pMt2;
    *ppAltMt = pAltMt;
    *ppAltMt2 = pAltMt2;

    return 1;
}
bool SceneSubj::lineWidthIsImportant(int primitiveType) {
    if (primitiveType == GL_TRIANGLES) return false;
    if (primitiveType == GL_TRIANGLE_STRIP) return false;
    if (primitiveType == GL_TRIANGLE_FAN) return false;
    return true;
}
int SceneSubj::executeDJstandard(DrawJob* pDJ,
    float* uMVP, float* uMV3x3, float* uMM, float* uMVP4dm,
    float* uVectorToLight, float* uVectorToTranslucent, float* uCameraPosition, float line1pixSize, Material* pMt) {

    if (pMt == NULL)
        pMt = &(pDJ->mt);

    int shaderN = pMt->shaderN;
    if (shaderN < 0)
        return 0;

    if (lineWidthIsImportant(pMt->primitiveType)) {
        float lw = line1pixSize * pMt->lineWidth;
        if (lw < 0.5)
            return 0;
        glLineWidth(lw);
    }

    Shader* pShader = Shader::shaders.at(shaderN);
    glUseProgram(pShader->GLid);

    //input uniform matrices
    if (pShader->l_uMVP >= 0)
        glUniformMatrix4fv(pShader->l_uMVP, 1, GL_FALSE, (const GLfloat*)uMVP);
    if (pShader->l_uMV3x3 >= 0)
        glUniformMatrix3fv(pShader->l_uMV3x3, 1, GL_FALSE, (const GLfloat*)uMV3x3);
    if (pShader->l_uMM >= 0)
        glUniformMatrix4fv(pShader->l_uMM, 1, GL_FALSE, (const GLfloat*)uMM);
    //shadow uniform matrix
    if (pShader->l_uTex4dm >= 0)
        glUniformMatrix4fv(pShader->l_uMVP4dm, 1, GL_FALSE, (const GLfloat*)uMVP4dm);

    executeDJcommon(pDJ, pShader, uVectorToLight, uVectorToTranslucent, uCameraPosition, pMt);
    return 1;
}
int SceneSubj::executeDJcommon(DrawJob* pDJ, Shader* pShader,
    float* uVectorToLight, float* uVectorToTranslucent, float* uCameraPosition, Material* pMt) {
    //assuming that shader program set, transform matrices provided
    glBindVertexArray(pDJ->glVAOid);

    if (pShader->l_uDepthBias >= 0) //for depthMap
        glUniformMatrix4fv(pShader->l_uDepthBias, 1, GL_FALSE, (const GLfloat*)Shadows::uDepthBias);

    //shadow uniforms
    if (pShader->l_uTex4dm >= 0) {
        //translate to texture
        int textureId = Texture::getGLid(Shadows::depthMapTexN);
        //pass textureId to shader program
        glActiveTexture(GL_TEXTURE4); // activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, textureId);
        // Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 4.
        glUniform1i(pShader->l_uTex4dm, 4);

        glUniform1f(pShader->l_uShadow, Shadows::shadowLight);
    }

    if (pShader->l_uVectorToLight >= 0)
        glUniform3fv(pShader->l_uVectorToLight, 1, (const GLfloat*)uVectorToLight);

    if (pShader->l_uTranslucency >= 0) {
        glUniform1f(pShader->l_uTranslucency, pMt->uTranslucency);
        if (pMt->uTranslucency > 0)
            glUniform3fv(pShader->l_uVectorToTranslucent, 1, (const GLfloat*)uVectorToTranslucent);
    }

    if (pShader->l_uCameraPosition >= 0)
        glUniform3fv(pShader->l_uCameraPosition, 1, (const GLfloat*)uCameraPosition);

    //attach textures
    if (pShader->l_uTex0 >= 0) {
        int textureId = Texture::getGLid(pMt->uTex0);
        //pass textureId to shader program
        glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, textureId);
        // Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 0.    
        glUniform1i(pShader->l_uTex0, 0);
    }
    if (pShader->l_uTex1mask >= 0) {
        int textureId = Texture::getGLid(pMt->uTex1mask);
        //pass textureId to shader program
        glActiveTexture(GL_TEXTURE1); // activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, textureId);
        // Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 1.    
        glUniform1i(pShader->l_uTex1mask, 1);
    }
    if (pShader->l_uTex2nm >= 0) {
        int textureId = Texture::getGLid(pMt->uTex2nm);
        //pass textureId to shader program
        glActiveTexture(GL_TEXTURE2); // activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, textureId);
        // Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 2.    
        glUniform1i(pShader->l_uTex2nm, 2);
    }
    if (pShader->l_uTex0translateChannelN >= 0) {
        glUniform1i(pShader->l_uTex0translateChannelN, pMt->uTex0translateChannelN);
        if (pMt->uTex0translateChannelN >= 4) {
            //translate to 2-tone
            if (pShader->l_uColor1 >= 0) {
                glUniform4fv(pShader->l_uColor1, 1, pMt->uColor1.forGL());
                glUniform4fv(pShader->l_uColor2, 1, pMt->uColor2.forGL());
            }
        }
        else
            if (pShader->l_uTex3 >= 0 && pMt->uTex3 >= 0) {
                //translate to texture
                int textureId = Texture::getGLid(pMt->uTex3);
                //pass textureId to shader program
                glActiveTexture(GL_TEXTURE3); // activate the texture unit first before binding texture
                glBindTexture(GL_TEXTURE_2D, textureId);
                // Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 3.    
                glUniform1i(pShader->l_uTex3, 3);
            }
    }
    //tex coords modifiers
    if (pShader->l_uTexMod >= 0)
        glUniform4fv(pShader->l_uTexMod, 1, pMt->uTexMod);

    //for normal map
    if (pShader->l_uHaveBinormal >= 0)
        glUniform1i(pShader->l_uHaveBinormal, pDJ->uHaveBinormal);

    //material uniforms
    if (pShader->l_uTex1alphaChannelN >= 0)
        glUniform1i(pShader->l_uTex1alphaChannelN, pMt->uTex1alphaChannelN);
    if (pShader->l_uTex1alphaNegative >= 0)
        glUniform1i(pShader->l_uTex1alphaNegative, pMt->uTex1alphaNegative);
    if (pShader->l_uColor >= 0)
        glUniform4fv(pShader->l_uColor, 1, pMt->uColor.forGL());
    if (pShader->l_uAlphaFactor >= 0)
        glUniform1f(pShader->l_uAlphaFactor, pMt->uAlphaFactor);
    if (pShader->l_uAlphaBlending >= 0)
        glUniform1i(pShader->l_uAlphaBlending, pMt->uAlphaBlending);
    if (pShader->l_uAmbient >= 0)
        glUniform1f(pShader->l_uAmbient, pMt->uAmbient);
    if (pShader->l_uSpecularIntencity >= 0)
        glUniform1f(pShader->l_uSpecularIntencity, pMt->uSpecularIntencity);
    if (pShader->l_uSpecularMinDot >= 0)
        glUniform2fv(pShader->l_uSpecularMinDot, 1, pMt->uSpecularMinDot);
    if (pShader->l_uSpecularPowerOf >= 0)
        glUniform1f(pShader->l_uSpecularPowerOf, pMt->uSpecularPowerOf);
    if (pShader->l_uBleach >= 0)
        glUniform1f(pShader->l_uBleach, pMt->uBleach);
    if (pShader->l_uShadingK >= 0)
        glUniform1f(pShader->l_uShadingK, pMt->uShadingK);
    if (pShader->l_uDiscardNormalsOut >= 0)
        glUniform1i(pShader->l_uDiscardNormalsOut, pMt->uDiscardNormalsOut);
    if (pShader->l_uFogLevel >= 0)
        glUniform1f(pShader->l_uFogLevel, pMt->uFogLevel);
    if (pShader->l_uFogColor >= 0)
        glUniform4fv(pShader->l_uFogColor, 1, pMt->uFogColor.forGL());

    //adjust render settings
    if (pMt->zBuffer > 0) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }
    else
        glDisable(GL_DEPTH_TEST);

    if (pMt->zBufferUpdate > 0)
        glDepthMask(GL_TRUE);
    else
        glDepthMask(GL_FALSE);

    if (pShader->l_uAlphaBlending >= 0 && pMt->uAlphaBlending > 0 || pMt->uAlphaFactor < 1) {
        glDepthMask(GL_FALSE); //don't update z-buffer
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
        glDisable(GL_BLEND);

    //execute
    if (pDJ->glEBOid > 0)
        glDrawElements(pMt->primitiveType, pDJ->pointsN, GL_UNSIGNED_SHORT, 0);
    else
        glDrawArrays(pMt->primitiveType, 0, pDJ->pointsN);

    glBindVertexArray(0);


    //checkGLerrors("DrawJob::executeDrawJob end");

    return 1;
}

void SceneSubj::fillBBox(Gabarites* pGB, SceneSubj* pSS0, mat4x4 mMVP) {
    //calculate mid point position
    v3toGL(pGB->bbMid, mMVP, pSS0->gabaritesOnLoad.bbMid, 1);
    //build size vectors
    float sizeVector[3][4];
     for (int axisN = 0; axisN < 3; axisN++) {
        if (pSS0->gabaritesOnLoad.bbRad[axisN] == 0) {
            v3setAll(sizeVector[axisN], 0);
            v3setAll(pGB->sizeDirXY[axisN], 0);
            pGB->sizeXY[axisN] = 0;
            continue;
        }
        float vIn[4];
        v3copy(vIn, pSS0->gabaritesOnLoad.bbMid);
        vIn[axisN] += pSS0->gabaritesOnLoad.bbRad[axisN];
        float vOut[4];
        v3toGL(vOut, mMVP, vIn, 1);

        v3fromTo(sizeVector[axisN], pGB->bbMid, vOut);
        v3normXY(pGB->sizeDirXY[axisN], sizeVector[axisN]);
        pGB->sizeXY[axisN] = v3lengthXY(sizeVector[axisN]);
    }
    //calculate on-screen bounding box
    v3setAll(pGB->bbMin, 1000000);
    v3setAll(pGB->bbMax, -1000000);
    for (int z = -1; z <= 1; z += 2)
        for (int y = -1; y <= 1; y += 2)
            for (int x = -1; x <= 1; x += 2)
                for (int i = 0; i < 3; i++) {
                    float xx = pGB->bbMid[i] + sizeVector[2][i] * z + sizeVector[1][i] * y + sizeVector[0][i] * x;
                    if (pGB->bbMin[i] > xx)
                        pGB->bbMin[i] = xx;
                    if (pGB->bbMax[i] < xx)
                        pGB->bbMax[i] = xx;
                }
}


