#ifndef SCENEMANAGER_HPP
#define SCENEMANAGER_HPP

#include <scene.hpp>
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
	

	// render
	void render();

private:
	//RenderQueues renderQueues;
	glm::vec4 backgroundColor;
	SceneData sceneData;
	Buffer sceneDataCB;
	const Camera *camera = nullptr;
	Renderer *renderer = nullptr;
};

 
#endif /* end of include guard: SCENEMANAGER_HPP */