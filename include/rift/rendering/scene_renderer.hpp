#ifndef SCENE_RENDERER_HPP
#define SCENE_RENDERER_HPP

#include <scene/scene.hpp>
#include <rendering/opengl4.hpp>

struct NVGcontext; 

struct SceneView
{
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
	glm::mat4 viewProjMatrix;
	glm::vec4 lightDir;
	glm::vec4 wEye;	// in world space
	glm::vec2 viewportSize;
};

struct ShadowPass
{
	SceneView *sceneView;
	Buffer *sceneViewUBO;
};

struct ForwardPass
{
	ForwardPass() = default;
	SceneView *sceneView = nullptr;
	Buffer *sceneViewUBO = nullptr;
	Light *light = nullptr;
	Buffer *lightParamsUBO = nullptr;
	// TODO shadow maps?
	Material *lastMaterial = nullptr;
	GLuint lastProgram = 0;
};

class SceneRenderer
{
public:
	SceneRenderer(glm::ivec2 viewportSize_, GraphicsContext &gc, AssetDatabase &assetDb);

	void setSceneCamera(const Camera &camera);

	//===========================================================
	void renderScene(Scene &scene, float dt);
	// add a mesh to render list (render this frame only)
	void drawMesh(const Transform &transform, Mesh &mesh, Material &material);

	//===========================================================
	// immediate mode

	// draw a mesh in wireframe from data on the CPU
	// A camera must be set before
	void drawWireMesh(
		const Transform &transform,
		GLenum mode,
		const util::array_ref<glm::vec3> vertices,
		const util::array_ref<uint16_t> indices,
		const glm::vec4 lineColor,
		bool noDepthTest = false);
	void drawWireMesh(
		const Transform &transform,
		GLenum mode,
		Buffer *vertices,
		unsigned nbvertices,
		Buffer *indices,
		unsigned nbindices,
		const glm::vec4 lineColor,
		bool noDepthTest = false);

	// draw a line 
	void drawLine();
	// draw text on top
	void drawText(glm::ivec2 pos, const char *str);
	// draw text with custom font
	void drawText2(
		glm::ivec2 pos,
		const char *str,
		const Font &font,
		const glm::vec4 &color,
		const glm::vec4 &outlineColor);

private:
	void init();

	void prepareMaterialForwardPass(
		Material &mat,
		ForwardPass &pass, 
		const glm::mat4 &modelToWorld);

	void drawMeshForwardPass(
		ForwardPass &pass,
		Mesh &mesh,
		Material &material,
		const glm::mat4 &modelToWorld);

	void makeTextVBO(Font &font, const char *str, Buffer *&vb, Buffer *& ib, unsigned &len);
	void drawTextVBO(Buffer *vb, Buffer *ib, unsigned len, const Font &font, glm::ivec2 pos, glm::vec4 fill, glm::vec4 outline);
	void drawTextShadow(glm::ivec2 pos, const char *str);

	void drawScreenMessages();
	void drawFrameTimeGraph(Scene &scene);

	void drawTerrain(ForwardPass &pass, Terrain &terrain);

	Camera camera;
	glm::ivec2 viewportSize;
	GraphicsContext &graphicsContext;
	NVGcontext *nvgContext;
	Material::Ptr defaultMaterial;
	VAO meshVao;
	VAO textVao;
	VAO immediateVao;
	GLuint dummy_vao;
	GLuint textProgram;
	GLuint immediateProgram;
	GLuint postprocProgram;
	Font::Ptr defaultFont;

	// Terrain rendering
	VAO terrainVao;
	GLuint terrainProgram;

	// PostProc render targets (ping-pong FBOs)
	// framebuffer
	GLuint rt0_fb;
	// color texture
	GLuint rt0_color;
	// depth texture
	GLuint rt0_depth;
};

 
#endif /* end of include guard: SCENE_RENDERER_HPP */