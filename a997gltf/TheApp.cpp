#include "TheApp.h"
#include "platform.h"
#include "utils.h"
#include "linmath.h"
#include "Texture.h"
#include "DrawJob.h"
#include "ModelBuilder.h"
#include "TexCoords.h"
#include "ModelLoader.h"
#include "model_car/CarWheel.h"
#include "Shadows.h"
#include "ProgressBar.h"
#include "gltf/GLTFparser.h"
#include "gltf/GLTFskinShader.h"

extern std::string filesRoot;
extern float degrees2radians;

std::vector<SceneSubj*> TheApp::sceneSubjs;

int TheApp::getReady() {

    Shader::loadBasicShaders();
    UISubj::init();

    ProgressBar* pPBar = new ProgressBar(filesRoot + "/dt/cfg/progressbar00.bin");

    bExitApp = false;
    frameN = 0;
    Shader::loadShaders(pPBar);
    GLTFskinShader::loadGLTFshaders(pPBar);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);//default
    glFrontFace(GL_CCW);//default

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);

    pTable = new TheTable(100, 10, 2, 4, 4);
    /*
    int subjN = GLTFparser::loadModel(&sceneSubjs, NULL, NULL, filesRoot + "/dt/models/people/999_oldman.glb", "", pPBar);
    //int subjN = ModelLoader::loadModel(&sceneSubjs, NULL, NULL, "/dt/test01.txt", "", pPBar);
    //int subjN = ModelLoader::loadModel(&sceneSubjs, NULL, NULL, "/dt/models/cars/999_1935_deusenberg_ssj/root01.txt", "",pPBar);
    //int subjN = ModelLoader::loadModel(&sceneSubjs, NULL, NULL, "/dt/models/cars/998_1929_deusenberg_j/root01_gatsby.txt", "",pPBar);
    SceneSubj* pSS0 = sceneSubjs.at(subjN);
    pSS0->scaleMe(40);
    pSS0->ownSpeed.setEulerDg(0,1,0);
    //pSS0->ownCoords.setEulerDg(0,-90,0);
    mat4x4_from_quat(pSS0->ownCoords.rotationMatrix, pSS0->ownCoords.getRotationQuat());
    pSS0->setRoot(pSS0);
    pSS0->onDeploy();
    */
    float scale = 64.0f / 150;
    int subjN = ModelLoader::loadModel(&sceneSubjs, NULL, NULL, "/dt/models/cars/999_1935_deusenberg_ssj/root01.txt", "", pPBar);
    SceneSubj* pSS = sceneSubjs.at(subjN);
    pSS->scaleMe(scale);
    v3set(pSS->ownCoords.pos, 4, pTable->groundLevel, -10);
    pSS->ownCoords.setEulerDg(0, 20, 0);
    mat4x4_from_quat(pSS->ownCoords.rotationMatrix, pSS->ownCoords.getRotationQuat());
    pSS->setRoot(pSS);
    pSS->onDeploy();

    subjN = ModelLoader::loadModel(&sceneSubjs, NULL, NULL, "/dt/models/misc/999_marlboro01red/root01.txt", "", pPBar);
    pSS = sceneSubjs.at(subjN);
    pSS->scaleMe(1.0 / 130.0);
    v3set(pSS->ownCoords.pos, 2.2, pTable->groundLevel + 6.13, 1.1);
    pSS->ownCoords.setEulerDg(0, -90, 0);
    mat4x4_from_quat(pSS->ownCoords.rotationMatrix, pSS->ownCoords.getRotationQuat());
    pSS->setRoot(pSS);

    subjN = ModelLoader::loadModel(&sceneSubjs, NULL, NULL, "/dt/models/misc/998_dollar/01/root01.txt", "", pPBar);
    pSS = sceneSubjs.at(subjN);
    pSS->scaleMe(1.0 / 130.0);
    v3set(pSS->ownCoords.pos, -2.5, pTable->groundLevel+0.01, 0.9);
    pSS->ownCoords.setEulerDg(0, -50, 0);
    mat4x4_from_quat(pSS->ownCoords.rotationMatrix, pSS->ownCoords.getRotationQuat());
    pSS->setRoot(pSS);


    int rootNodeN = GLTFparser::loadModel(&sceneSubjs, NULL, NULL, filesRoot + "/dt/models/people/999_oldman.glb", "", pPBar);
    GLTFparser::loadAnimations(filesRoot + "/dt/models/people/999_oldman.glb", pPBar);
    pSS = sceneSubjs.at(rootNodeN);
    //pSS->pStickTo = pSS0;
    pSS->scaleMe(7);// 6.67);
    v3set(pSS->ownCoords.pos, -4, pTable->groundLevel, 0);
    pSS->ownCoords.setEulerDg(0, -30, 0);
    GLTFnode* pN0 = (GLTFnode*)sceneSubjs.at(rootNodeN + 1);
    GLTFnode::startAnimation(pN0, 0, 2); // 2-back-and-forth
    //pSS->ownSpeed.setEulerDg(0, 0.5, 0);

    //===== set up light
    v3set(dirToMainLight, -1, 1, 0.5);
    //v3set(dirToMainLight, 0, 1, 0.5);
    v3norm(dirToMainLight);

    Shadows::init();
    //===== set up camera
    mainCamera.ownCoords.setEulerDg(40, 160, 0); //set camera angles/orientation
    mat4x4_from_quat(mainCamera.ownCoords.rotationMatrix, mainCamera.ownCoords.getRotationQuat());

    mainCamera.viewRangeDg = 15;// 20;
    mainCamera.stageSize[0] = 17;// 85;
    mainCamera.stageSize[1] = 12;// 45;
    //memcpy(mainCamera.lookAtPoint, pGS->ownCoords.pos, sizeof(float) * 3);
    mainCamera.lookAtPoint[1] = pTable->groundLevel+6;
    mainCamera.reset(&mainCamera, &pTable->worldBox);

    pPBar->nextStep(pPBar);
    UISubj::cleanUp();


    //=============arrays to process
    pSubjArrays2draw.push_back(&sceneSubjs);
    pSubjArrays2draw.push_back(&pTable->tableParts);
    //======= set UI
    //UISubj::addZBufferSubj("test", 20, 10, 600, 600, "top left", Shadows::depthMapTexN);

     return 1;
}


int TheApp::drawFrame() {
    myPollEvents(); 

    //scan subjects
    int subjsN = sceneSubjs.size();
    for (int subjN = 0; subjN < subjsN; subjN++) {
        SceneSubj* pSS = sceneSubjs.at(subjN);
        if (pSS == NULL)
            continue;
        if (pSS->hide > 0)
            continue;

        pSS->processSubj();

        int a = 0;
    }

    //translucent light
    float vecToLightInEyeSpace[4];
    mat4x4_mul_vec4plus(vecToLightInEyeSpace, (vec4*)mainCamera.mViewProjection, dirToMainLight, 0);
    float dirToTranslucent[3];
    if (vecToLightInEyeSpace[2] <= 0) { //light from behind
        for (int i = 0; i < 3; i++) {
            if (i == 2)
                dirToTranslucent[i] = dirToMainLight[i];
            else
                dirToTranslucent[i] = -dirToMainLight[i];
        }
    }
    else { //light to eyes
        for (int i = 0; i < 3; i++) {
            dirToTranslucent[i] = -dirToMainLight[i];
        }
    }
    dirToTranslucent[2] *= 0.5;
    v3norm(dirToTranslucent);

    std::vector<std::vector<SceneSubj*>*> pSubjArrays2draw;
    pSubjArrays2draw.push_back(&sceneSubjs);
    pSubjArrays2draw.push_back(&pTable->tableParts);
    int arraysN = pSubjArrays2draw.size();
    for (int nA = 0; nA < arraysN; nA++) {
        std::vector<SceneSubj*>* pSubjs = pSubjArrays2draw.at(nA);
        int subjsN = pSubjs->size();
        //calculate screen coords/gabarites
        for (int subjN = 0; subjN < subjsN; subjN++) {
            SceneSubj* pSS = pSubjs->at(subjN);
            if (pSS == NULL)
                continue;
            if (pSS->hide > 0)
                continue;
            pSS->getScreenPosition(pSS, mainCamera.mViewProjection);// , & mainCamera, dirToMainLight);
        }
    }

    Shadows::addToShadowsQueue(&sceneSubjs);
    Shadows::renderDepthMap();
			checkGLerrors("after renderDepthMap");
 
    //set render to screen
    glViewport(0, 0, mainCamera.targetSize[0], mainCamera.targetSize[1]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDepthMask(GL_TRUE);
    //glClearColor(0.0, 0.0, 0.0, 1.0);
    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //sort for rendering
    std::vector<SceneSubj*> renderQueueOpaque;
    std::vector<SceneSubj*> renderQueueTransparent;
    arraysN = pSubjArrays2draw.size();
    for (int nA = 0; nA < arraysN; nA++) {
        std::vector<SceneSubj*>* pSubjs = pSubjArrays2draw.at(nA);
        SceneSubj::addToRenderQueue(&renderQueueOpaque, &renderQueueTransparent, pSubjs);
    }
    //draw opaque subjects
    SceneSubj::sortRenderQueue(&renderQueueOpaque, 0); //0-closer 1st
    subjsN = renderQueueOpaque.size();
		//mylog("frame%d, %d opaques\n",frameN,subjsN);
    for (int subjN = 0; subjN < subjsN; subjN++) {
        SceneSubj* pSS = renderQueueOpaque.at(subjN);
        pSS->render(&mainCamera, dirToMainLight, dirToTranslucent, 0); //0 - render opaque only
 			checkGLerrors("after lrender opaque");
   }
    renderQueueOpaque.clear();

    //draw transparent subjects
    SceneSubj::sortRenderQueue(&renderQueueTransparent, 1); //1-farther 1st
    subjsN = renderQueueTransparent.size();
		//mylog("%d tranparents\n",subjsN);
    for (int subjN = 0; subjN < subjsN; subjN++) {
        SceneSubj* pSS = renderQueueTransparent.at(subjN);
        pSS->render(&mainCamera, dirToMainLight, dirToTranslucent, 1); //1 - render transparent only
 			checkGLerrors("after lrender transparent");
    }
    renderQueueTransparent.clear();

    UISubj::renderAll();
			//mylog("after render UI\n");
			

    //synchronization
    while (1) {
        uint64_t currentMillis = getSystemMillis();
        uint64_t millisSinceLastFrame = currentMillis - lastFrameMillis;
        if (millisSinceLastFrame >= millisPerFrame) {
            lastFrameMillis = currentMillis;
            break;
        }
    }
    mySwapBuffers();
    frameN++;
    return 1;
}

int TheApp::cleanUp() {
    int itemsN = sceneSubjs.size();
    //delete all UISubjs
    for (int i = 0; i < itemsN; i++)
        delete sceneSubjs.at(i);
    sceneSubjs.clear();
    //clear all other classes
    MaterialAdjust::cleanUp();
    Texture::cleanUp();
    Shader::cleanUp();
    DrawJob::cleanUp();
    MyColor::cleanUp();
    UISubj::cleanUp();
    return 1;
}
int TheApp::onScreenResize(int width, int height) {
			//mylog("onScreenResize %d x %d\n",width,height);
    if (mainCamera.targetSize[0] == width && mainCamera.targetSize[1] == height)
        return 0;
    if(pTable != NULL)
        mainCamera.onTargetResize(&mainCamera, width, height,&pTable->worldBox);
    UISubj::onScreenResize(width, height);
    mylog(" screen size %d x %d\n", width, height);
    return 1;
}

SceneSubj* TheApp::newSceneSubj(std::string subjClass, std::string sourceFile,
    std::vector<SceneSubj*>* pSubjsSet0, std::vector<DrawJob*>* pDrawJobs0) {
    SceneSubj* pSS = NULL;
    if (subjClass.compare("") == 0)
        pSS = (new SceneSubj());
    else if (subjClass.find("Car") == 0) {
        if (subjClass.compare("CarWheel") == 0)
            pSS = (new CarWheel());
    }
    if (pSS == NULL) {
        mylog("ERROR in TheProject::newSceneSubj. %s class not found\n", subjClass.c_str());
        return NULL;
    }
    strcpy_s(pSS->className, 32, subjClass.c_str());
    strcpy_s(pSS->source, 256, sourceFile.c_str());
    if (pDrawJobs0 != NULL)
        pSS->pDrawJobs = pDrawJobs0;
    if (pSubjsSet0 != NULL) {
        pSS->nInSubjsSet = pSubjsSet0->size();
        pSS->pSubjsSet = pSubjsSet0;
        pSubjsSet0->push_back(pSS);
    }
    return pSS;
}

int TheApp::run() {
    getReady();
    while (!bExitApp) {
        drawFrame();
    }
    cleanUp();
    return 1;
}
