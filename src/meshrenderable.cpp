#include <meshrenderable.hpp>
#include <log.hpp>

void MeshRenderable::render(RenderContext const &context) 
{
	//LOG << "MeshRenderable::render";

	context.renderer->setVertexBuffer(0, mMesh->getVertexBuffer());
	context.renderer->setIndexBuffer(mMesh->getIndexBuffer());
	context.renderer->setVertexLayout(Mesh::getSharedVertexLayout());
	context.renderer->setConstantBuffer(0, context.perFrameShaderParameters);
	context.renderer->setNamedConstantMatrix4("modelMatrix", mEntity->getTransform().toMatrix());

	// TODO:
	// mesh->prepare(rendercontext)
	// material->prepare(rendercontext)
	
	const int n = mMesh->getNumSubMeshes();
	for (int i = 0; i < n; ++i) {
		auto &&subMesh = mMesh->getSubMesh(i);
		context.renderer->drawIndexed(
			subMesh.mPrimitiveType, 
			subMesh.mVertexStartOffset, 
			subMesh.mNumVertices, 
			subMesh.mIndexStartOffset, 
			subMesh.mNumIndices);
	}

}

void MeshRenderable::init() 
{
}

void MeshRenderable::update(float dt) 
{
}

void MeshRenderable::setMesh(Mesh *mesh)
{
	mesh->addRef();
	mMesh = mesh;
}