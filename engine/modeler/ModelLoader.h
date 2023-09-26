#pragma once
#include "XMLparser.h"
#include "ModelBuilder.h"
#include "GroupTransform.h"
#include "MaterialAdjust.h"
#include "ProgressBar.h"

class ModelLoader : public XMLparser
{
public:
	ModelBuilder* pRootModelBuilder = NULL;
	ModelBuilder* pModelBuilder = NULL;
	bool ownModelBuilder = false;
	std::vector<SceneSubj*>* pSubjsVector = NULL;
	std::vector<DrawJob*>* pDrawJobs = NULL;
	std::vector<unsigned int>* pBuffersIds = NULL;

	MaterialAdjust* pMaterialAdjust = NULL;
	int lineStartsAt = -1;

	ProgressBar* pPBar=NULL;
public:
	ModelLoader(std::vector<SceneSubj*>* pSubjsVector0, std::vector<DrawJob*>* pDrawJobs0, std::vector<unsigned int>* pBuffersIds0,
		int subjN, ModelBuilder* pMB00, ModelBuilder* pMB, std::string filePath, ProgressBar* pPBar0) : XMLparser(filePath) {

		pSubjsVector = pSubjsVector0;
		pPBar = pPBar0;

		if(pDrawJobs0==NULL)
			pDrawJobs = &DrawJob::drawJobs_default;
		else
			pDrawJobs = pDrawJobs0;

		if(pBuffersIds0==NULL)
			pBuffersIds = &DrawJob::buffersIds_default;
		else
			pBuffersIds = pBuffersIds0;

		if (pMB != NULL) {
			ownModelBuilder = false;
			pModelBuilder = pMB;
		}
		else {
			ownModelBuilder = true;
			pModelBuilder = new ModelBuilder();
			pModelBuilder->lockGroup(pModelBuilder);
		}
		if(subjN >= 0)
			pModelBuilder->useSubjN(pModelBuilder, subjN);

		if (pMB00 != NULL)
			pRootModelBuilder = pMB00;
		else
			pRootModelBuilder = pModelBuilder;
	};
	virtual ~ModelLoader() {
		if (!ownModelBuilder)
			return;
		pModelBuilder->buildDrawJobs(pModelBuilder, pSubjsVector, pDrawJobs, pBuffersIds);
		delete pModelBuilder;
		return;
	};
	virtual int processTag() { return processTag(this); };
	static int processTag(ModelLoader* pML);
	static int processTag_a(ModelLoader* pML); //apply
	static int setIntValueFromHashMap(int* pInt, std::map<std::string, float> floatsHashMap, std::string varName, std::string tagStr);
	static int setFloatValueFromHashMap(float* pFloat, std::map<std::string, float> floatsHashMap, std::string varName, std::string tagStr);
	static int setTexture(ModelLoader* pML, int* pInt, std::string txName);
	static int setMaterialTextures(ModelLoader* pML, Material* pMT);
	static int fillProps_vs(VirtualShape* pVS, std::map<std::string, float> floatsHashMap, std::string tagStr); //virtual shape
	static int fillProps_mt(Material* pMT, std::string tagStr, ModelLoader* pML); //Material
	static int fillProps_gt(GroupTransform* pGS, ModelBuilder* pMB, std::string tagStr);
	static int loadModel(std::vector<SceneSubj*>* pSubjsVector0, std::vector<DrawJob*>* pDrawJobs0, std::vector<unsigned int>* pBuffersIds0,
		std::string sourceFile, std::string subjClass, ProgressBar* pPBar, ModelBuilder* pMB00 = NULL);
	static int processTag_clone(ModelLoader* pML);
	static int addMark(char* marks, std::string newMark);
	static int processTag_do(ModelLoader* pML);
	static int processTag_a2mesh(ModelLoader* pML);
	static int processTag_line2mesh(ModelLoader* pML);
	static int processTag_points2mesh(ModelLoader* pML);
	static int processTag_element(ModelLoader* pML);
	static int processTag_include(ModelLoader* pML);
	static int processTag_short(ModelLoader* pML);
	static int processTag_lineStart(ModelLoader* pML);
	static int processTag_lineEnd(ModelLoader* pML);
	static int processTag_a2group(ModelLoader* pML);
	static int processTag_color_as(ModelLoader* pML);
	static int processTag_lastLineTexure(ModelLoader* pML);
	static int processTag_group2line(ModelLoader* pML);
	static int processTag_lineTip(ModelLoader* pML);
	static int processTag_model(ModelLoader* pML);
	static int buildGabaritesFromDrawJobs(Gabarites* pGB, std::vector<DrawJob*>* pDrawJobs, int djStartN, int djTotalN);

};

