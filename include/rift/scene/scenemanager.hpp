#ifndef SCENEMANAGER_HPP
#define SCENEMANAGER_HPP

#include <submission.hpp>
#include <renderqueue.hpp>
#include <camera.hpp>
#include <mesh.hpp>


//
// SceneManager
// Gère le rendu d'une scène, à savoir:
//  - l'ordre de rendu des éléments
//  - la génération des shadow maps
//  - l'application des effets de post-processing
//  - la caméra
//  - les lumières
//  - TODO viewports
class SceneManager
{
public:
	// données partagées entre les shaders
	struct FrameData
	{
		glm::mat4 viewMatrix;
		glm::mat4 projMatrix;
		glm::mat4 viewProjMatrix;
		glm::vec4 eyePos;	// in world space
		glm::vec4 lightDir;
		glm::vec2 viewportSize;
	};

	// non default constructible
	SceneManager(Renderer &renderer, const Camera &camera);
	// noncopyable
	SceneManager(const SceneManager &) = delete;
	SceneManager &operator=(const SceneManager &) = delete;
	// nonmoveable
	SceneManager(SceneManager &&) = delete;
	SceneManager &operator=(SceneManager &&rhs) = delete;

	// change camera
	SceneManager &setCamera(const Camera &camera);
	// background color
	SceneManager &setBackgroundColor(const glm::vec4& color);
	
	// submit mesh for rendering
	SceneManager &draw(
		const Mesh &mesh, 
		unsigned int startVertex, 
		unsigned int startIndex, 
		unsigned int numIndices,
		const Material &material,
		const Transform &transform);

	// raw submission
	SceneManager &draw(const Submission &submission);

	// render and flush render queues
	void render();

private:
	RenderQueue mRenderQueue;
	FrameData mFrameData;
	glm::vec4 mBackgroundColor;
	ConstantBuffer *mFrameDataCB = nullptr;
	const Camera *mCamera = nullptr;
	Renderer *mRenderer = nullptr;
};

 
#endif /* end of include guard: SCENEMANAGER_HPP */