#ifndef MODEL_HPP
#define MODEL_HPP

#include <renderable.hpp>
#include <meshbuffer.hpp>
#include <material.hpp>

class CModel : public CRenderable
{
public:

	struct MeshPart {
		CMeshBuffer *mMeshBuffer;
		CMaterial *mMaterial;
	};

	void addMeshPart(CMeshBuffer *meshBuffer, CMaterial *material);
	void render(Transform &transform); 
	void deleteResource();


private:
	std::vector<MeshPart> mMeshParts;
};


#endif