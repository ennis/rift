#include <scenemanager.hpp>
#include <engine.hpp>

SceneManager::SceneManager(Renderer &renderer, const Camera &camera) :
mRenderer(&renderer),
mCamera(&camera)
{
	// create frame data CB
	mFrameDataCB = renderer.createConstantBuffer(
		sizeof(FrameData), 
		ResourceUsage::Dynamic, 
		nullptr);
}

SceneManager &SceneManager::draw(
	const Mesh &mesh, 
	unsigned int startVertex, 
	unsigned int startIndex, 
	unsigned int numIndices,
	const Material &material,
	const Transform &transform)
{
	Submission s {
		transform, 
		0, 0,	// viewport & bucket id
		&material,	// material
		mesh.getVertexBuffers(),	// VB
		mesh.getIndexBuffer(),	// IB
		mesh.getVertexLayout(),
		mesh.getPrimitiveType(),
		1,
		startVertex,
		1,	// dummy
		startIndex,
		numIndices
	};

	mRenderQueue.addSubmission(s);
	return *this;
}

SceneManager &SceneManager::setBackgroundColor(const glm::vec4& color)
{
	mBackgroundColor = color;
	return *this;
}

void SceneManager::render()
{
	auto &R = *mRenderer;
	auto &cam = *mCamera;
	// TODO
	// - create shadow maps

	// update frameData CB
	// render context
	auto viewportSize = Engine::instance().getWindow().size();
	// update constant buffer
	mFrameData.eyePos = glm::vec4(cam.getEntity()->getTransform().position, 0.0f);
	mFrameData.lightDir = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	mFrameData.projMatrix = cam.getProjectionMatrix();
	mFrameData.viewMatrix = cam.getViewMatrix();
	mFrameData.viewportSize = viewportSize;
	mFrameData.viewProjMatrix = mFrameData.projMatrix * mFrameData.viewMatrix;
	R.updateBuffer(mFrameDataCB, 0, sizeof(FrameData), &mFrameData);

	// render to screen
	R.bindRenderTargets(0, nullptr, nullptr);
	// update viewport
	// XXX hack
	Viewport vp{ 0.f, 0.f, float(viewportSize.x), float(viewportSize.y) };
	R.setViewports(1, &vp);
	// clear RT
	R.clearColor(mBackgroundColor);
	R.clearDepth(100.f);

	// render everything
	// TODO actually optimize things
	// TODO do this in render queue
	for (const auto &s : mRenderQueue.getSubmissions()) 
	{
		// set vertex buffers
		for (unsigned int i = 0; i < s.VB.size(); ++i) {
			R.setVertexBuffer(i, s.VB[i]);
		}
		R.setIndexBuffer(s.IB);
		R.setVertexLayout(s.vertexLayout);
		// setup material
		s.material->setup(R);
		// bind frame data
		R.setConstantBuffer(0, mFrameDataCB);
		// draw command
		R.drawIndexed(
			s.primitiveType, 
			s.startVertex,
			s.numVertices, 
			s.startIndex, 
			s.numIndices);
	}
}
