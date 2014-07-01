#ifndef MODEL_HPP
#define MODEL_HPP

#include <meshbuffer.hpp>
#include <material.hpp>
#include <renderer.hpp>

struct CModel : public CRenderResource
{
	struct MeshPart {
		// not owned by model
		CMeshBufferRef mMeshBuffer;
		// not owned by model
		CMaterialRef mMaterial;
	};

	void addMeshPart(CMeshBufferRef meshBuffer, CMaterialRef material);
	void setMaterial(int meshPart, CMaterialRef material);
	void destroy();

	std::vector<MeshPart> mMeshParts;
};



#endif