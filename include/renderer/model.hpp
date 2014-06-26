#ifndef MODEL_HPP
#define MODEL_HPP

#include <meshbuffer.hpp>
#include <material.hpp>
#include <renderer.hpp>

struct CModel : public CRenderResource
{
	struct MeshPart {
		CMeshBufferRef mMeshBuffer;
		CMaterialRef mMaterial;
	};

	void addMeshPart(CMeshBufferRef meshBuffer, CMaterialRef material);
	void destroy();

	std::vector<MeshPart> mMeshParts;
};



#endif