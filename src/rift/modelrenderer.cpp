#include <modelrenderer.hpp>


void ModelRenderer::render(const RenderContext &context, const Transform &modelTransform)
{
	assert(mModel);
	const auto &submeshes = mModel->getSubmeshes();
	for (unsigned int i = 0; i < submeshes.size(); ++i) {
		if (mMaterialMap[i] != nullptr) {
			auto mat = mMaterialMap[i];
			// setup material
			mat->setup(*context.renderer);
		}
		// setup model matrix
		context.renderer->setConstantBuffer(0, context.perFrameShaderParameters);
		context.renderer->setNamedConstantMatrix4("modelMatrix", modelTransform.toMatrix());
		// draw part of mesh
		const auto &mesh = mModel->getMesh();
		const auto &sm = submeshes[i];
		mesh.drawPart(sm.startVertex, sm.startIndex, sm.numIndices);
	}
}