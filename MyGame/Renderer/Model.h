#ifndef MODEL_H
#define MODEL_H


class noRenderModel {


protected:
	//noMesh* m_mesh;


};

class noRenderModelStatic : public noRenderModel {

};

class noRenderModelHkX : public noRenderModel {	

	void Load(const char* szFilename);
};

class noRenderModelPSK : public noRenderModel {

	void Load(const char* szFilename);
};

class noRenderModelBeam {


};

class noRenderModelTrail {


};

#endif