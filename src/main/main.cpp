#include <game.hpp>
#include <string>
#include <transform.hpp>
#include <log.hpp>
#include <entity.hpp>
#include <AntTweakBar.h>
#include <serialization.hpp>
#include <scene.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <image.hpp>
#include <sky.hpp>
#include <gl4/renderer.hpp>
#include <gl4/effect.hpp>	// Effect
#include <font.hpp>
#include <colors.hpp>
#include <small_vector.hpp>
#include <skeleton.hpp>
#include <animationcurve.hpp>
#include <skeletonanimation.hpp>
#include <resources.hpp>
#include <mesh.hpp>
#include <terrain.hpp>
#include <camera.hpp>
#include <textrenderer.hpp>
#include <nanovg/nanovg.h>
#include <nanovg/nanovg_backend.hpp>

/*class SkeletonDebug
{
public:
	static const unsigned kMaxJoints = 60;

	SkeletonDebug(Resources &resources)
	{
		shader = gl4::Effect::loadFromFile("resources/shaders/skeleton_debug.glsl")->compileShader();
		cylinder = resources.meshes.load("resources/models/sphere.mesh");
		cb_bone_transforms = Stream::create(BufferUsage::ConstantBuffer, sizeof(glm::mat4) * kMaxJoints, 3);
	}

	void drawSkeleton(
		const Skeleton &skeleton,
		const SkeletonAnimationSampler &animSampler,
		SceneRenderContext &context)
	{
		assert(skeleton.joints.size() < kMaxJoints);
		// compute pose
		auto pose = animSampler.getPose(skeleton, glm::mat4(1.0));
		for (size_t i = 0; i < pose.size(); i++)
		{
			pose[i] = pose[i] * glm::scale(glm::vec3(0.01*skeleton.joints[i].init_offset.length()));
		}

		// upload list of transforms
		auto ptr = cb_bone_transforms->reserve_many<glm::mat4>(kMaxJoints);
		memcpy(ptr, pose.data(), sizeof(glm::mat4)*pose.size());

		auto &renderQueue = *context.opaqueRenderQueue;
		renderQueue.beginCommand();
		renderQueue.setShader(*shader);
		renderQueue.setUniformBuffers({ context.sceneDataCB, cb_bone_transforms->getDescriptor() });
		cylinder->drawInstanced(renderQueue, 0, 0, pose.size());
		cb_bone_transforms->fence(renderQueue);
	}

private:
	Shader::Ptr shader;
	Mesh* cylinder;
	Stream::Ptr cb_bone_transforms;
};*/

//============================================================================
// Classe de base du jeu
class RiftGame : public Game
{
public:
	// Constructeur
	RiftGame()
	{
		init();
	}

	void init();
	void render(float dt);
	void update(float dt);
	void tearDown();
	void initTweakBar();
	void renderDebugHud();

private:
	float mLastTime = 0.f;
	float mTotalTime = 0.f;
	float mFPS = 0;
	float spinAngle = 0.f;
	unsigned long numFrames = 0;
	unsigned sizeX, sizeY;

	std::unique_ptr<Resources> resources;

	struct PerObject
	{
		glm::mat4 modelMatrix;
		glm::vec4 objectColor;
	};

	struct PerObjectPBR
	{
		glm::mat4 modelMatrix;
		glm::vec4 objectColor;
		float eta;
	};

	struct EnvCubeParams
	{
		glm::mat4 modelMatrix;
	};

	std::unique_ptr<TrackballCameraControl> trackball;
	Mesh::Ptr mesh;
	Mesh *mokou = nullptr;
	//SkinnedMesh *mokou_skin = nullptr;

	gl4::Effect::Ptr effect;
	gl4::Effect::Ptr effectEnvCube;
	gl4::Effect::Ptr effectPBR;

	Shader::Ptr shader;
	Shader::Ptr shaderWireframe;
	Shader::Ptr shaderPBR;
	Shader::Ptr shaderEnvCube;

	Texture2D *tex;
	TextureCubeMap *envmap;
	
	RenderTarget::Ptr shadowRT;
	RenderTarget::Ptr screenRT2;

	Shader::Ptr passthrough;

	struct FXParams
	{
		float thing;
		float vx_offset;
		float rt_w;
		float rt_h;
	};

	Font::Ptr font;
	Font::Ptr debugFont;
	std::unique_ptr<TextRenderer> hud;
	std::unique_ptr<Terrain> terrain;

	Sky sky;
	std::unique_ptr<Skeleton> skel;
	//std::unique_ptr<SkeletonDebug>  skel_debug;
	SkeletonAnimation skel_animation;
	std::unique_ptr<SkeletonAnimationSampler> skel_anim_sampler;

	util::ecs::world world;
	NVGcontext *nvg_context;
};


// entity test!

struct Position : public util::ecs::component<Position>
{
	Position(int x_, int y_) : x(x_), y(y_)
	{
		LOG << "CTOR Position";
	}

	~Position() 
	{
		LOG << "DTOR Position";
	}

	int x, y;
};

struct Health : public util::ecs::component<Health>
{
	Health(int hp_, int sp_) : hp(hp_), sp(sp_)
	{
		LOG << "CTOR Health";
	}
	
	~Health()
	{
		LOG << "DTOR Health";
	}

	int hp, sp;
};


//============================================================================
void RiftGame::init()
{
	//using namespace util::ecs;
	auto e = world.create_entity();

	world.add_component<Health>(e, 200, 10);
	world.add_component<Position>(e, 10, 10);
	world.get_component<Health>(e);
	world.add_component<Health>(e, 200, 10);
	world.add_component<Position>(e, 10, 10);

	world.remove_component<Health>(e);
	world.add_component<Health>(e, 300, 10);

	//w.delete_entity(e);
	
	resources = std::make_unique<Resources>();
	trackball = std::make_unique<TrackballCameraControl>(
		Engine::instance().getWindow(),
		glm::vec3{ 0.0f, 0.0f, -5.0f },
		45.0f,
		0.1,
		1000.0,
		0.01);

	// Effect 
	effect = gl4::Effect::loadFromFile("resources/shaders/default.glsl");
	shader = effect->compileShader();
	RasterizerDesc rs = {};
	rs.fillMode = PolygonFillMode::Wireframe;
	shaderWireframe = effect->compileShader({}, rs, DepthStencilDesc{});

	effectPBR = gl4::Effect::loadFromFile("resources/shaders/pbr.glsl");
	shaderPBR = effectPBR->compileShader();
	effectEnvCube = gl4::Effect::loadFromFile("resources/shaders/envcube.glsl");
	DepthStencilDesc envcube_ds;
	envcube_ds.depthTestEnable = true;
	envcube_ds.depthWriteEnable = false;
	shaderEnvCube = effectEnvCube->compileShader({}, RasterizerDesc{}, envcube_ds, BlendDesc{});

	// buffer contenant les données des vertex (c'est un cube, pour info)
	// ici: position (x,y,z), normales (x,y,z), coordonnées de texture (x,y) 
	// les normales et les coordonnées de textures sont fausses (c'est juste pour illustrer)
	static float cubeMeshData[] = {
		/* pos */ 0.0f, 0.0f, 1.0f, /* norm */ 0.0f, 0.0f, 1.0f, /* texcoord */ 0.0f, 1.0f,
		/* pos */ 1.0f, 0.0f, 1.0f, /* norm */ 0.0f, 0.0f, 1.0f, /* texcoord */ 0.0f, 1.0f,
		/* pos */ 1.0f, 1.0f, 1.0f, /* norm */ 0.0f, 0.0f, 1.0f, /* texcoord */ 0.0f, 1.0f,
		/* pos */ 0.0f, 1.0f, 1.0f, /* norm */ 0.0f, 0.0f, 1.0f, /* texcoord */ 0.0f, 1.0f,
		/* pos */ 0.0f, 0.0f, 0.0f, /* norm */ 0.0f, 0.0f, 1.0f, /* texcoord */ 0.0f, 1.0f,
		/* pos */ 1.0f, 0.0f, 0.0f, /* norm */ 0.0f, 0.0f, 1.0f, /* texcoord */ 0.0f, 1.0f,
		/* pos */ 1.0f, 1.0f, 0.0f, /* norm */ 0.0f, 0.0f, 1.0f, /* texcoord */ 0.0f, 1.0f,
		/* pos */ 0.0f, 1.0f, 0.0f, /* norm */ 0.0f, 0.0f, 1.0f, /* texcoord */ 0.0f, 1.0f,
	};

	// les indices (données de l'index buffer)
	static uint16_t cubeIndices[] = {
		0, 1, 2, 2, 3, 0,
		3, 2, 6, 6, 7, 3,
		7, 6, 5, 5, 4, 7,
		4, 0, 3, 3, 7, 4,
		0, 5, 1, 5, 0, 4,
		1, 5, 6, 6, 2, 1
	};

	Attribute attribs[] = {
		{ ElementFormat::Float3 },
		{ ElementFormat::Float3 },
		{ ElementFormat::Float2 },
	};

	Submesh sm { PrimitiveType::Triangle, 0, 0, 8, 36 };

	mesh = Mesh::create(
		attribs,
		8,
		cubeMeshData,
		36,
		cubeIndices,
		{ sm });

	mokou = resources->meshes.load("resources/models/rock/crystal.mesh");
	tex = resources->textures.load("resources/img/brick_wall.jpg");
	envmap = resources->cubeMaps.load("resources/img/env/uffizi/env.dds");
	envmap = resources->cubeMaps.load("resources/img/env/uffizi/env.dds");	// TEST

	glm::ivec2 win_size = Engine::instance().getWindow().size();
	shadowRT = RenderTarget::create(win_size, {}, ElementFormat::Depth16);

	debugFont = Font::loadFromFile("resources/img/fonts/debug.fnt");
	font = Font::loadFromFile("resources/img/fonts/arno_pro.fnt");
	hud = std::make_unique<TextRenderer>();

	DepthStencilDesc ds_fx;
	ds_fx.depthTestEnable = false;
	ds_fx.depthWriteEnable = false;
	passthrough = gl4::Effect::loadFromFile("resources/shaders/fxaa.glsl")->compileShader({}, RasterizerDesc{}, ds_fx, BlendDesc{});
	screenRT2 = RenderTarget::create({ 1280, 720 }, { ElementFormat::Unorm8x4 }, ElementFormat::Depth24);

	std::vector<BVHMapping> mappings;
	std::ifstream bvh("resources/models/animated/mokou_skeleton.bvh");
	skel = Skeleton::loadFromBVH(bvh, mappings);

	std::ifstream motion_file("resources/models/animated/mokou_run.bvh");
	//skel_debug = std::make_unique<SkeletonDebug>(*resources);
	skel_animation = SkeletonAnimation::loadFromBVH(motion_file, *skel, mappings);
	skel_anim_sampler = std::make_unique<SkeletonAnimationSampler>(*skel, skel_animation, 0.003f);
	skel_anim_sampler->nextFrame();

	/*terrain = std::make_unique<Terrain>(
		Image::loadFromFile("resources/img/terrain/test_heightmap_2.dds"),
		resources->textures.load("resources/img/mb_rocklface07_d.dds"),
		resources->textures.load("resources/img/grasstile_c.dds"));*/

	// TEST 
	//mokou_skin = resources->skinnedMeshes.load("resources/models/animated/mokou.mesh");
	nvg_context = nvg::backend::createContext();
}


void RiftGame::render(float dt)
{
	// rendu de la scene
	auto win_size = glm::ivec2(sizeX, sizeY);
	auto win_size_f = glm::vec2(sizeX, sizeY);
	auto cam = trackball->updateCamera();

	CommandBuffer opaqueList;
	SceneRenderContext context;
	context.opaqueList = &opaqueList;
	context.defaultFont = font.get();

	// update scene data buffer
	context.sceneData.eyePos = glm::vec4(cam.wEye, 1.0f);
	context.sceneData.projMatrix = cam.projMat;
	context.sceneData.viewMatrix = cam.viewMat;
	context.sceneData.viewProjMatrix = cam.projMat * cam.viewMat;
	context.sceneData.viewportSize = win_size;
	context.sceneData.lightDir = glm::vec4(0.f, 1.f, 0.f, 0.f);
	auto &sceneDataCB = Renderer::allocTransientBuffer(BufferUsage::ConstantBuffer, sizeof(SceneData));
	*(sceneDataCB.map_as<SceneData>()) = context.sceneData;
	context.sceneDataCB = &sceneDataCB;

	// update per-model buffer
	auto &cbPerObj = Renderer::allocTransientBuffer(BufferUsage::ConstantBuffer, sizeof(PerObject));
	auto cbPerObjPtr = cbPerObj.map_as<PerObject>();
	cbPerObjPtr->modelMatrix = glm::mat4(1.0f);
	cbPerObjPtr->objectColor = glm::vec4(1.0f);

	// update per-model buffer
	auto &cbPerObjPBR = Renderer::allocTransientBuffer(BufferUsage::ConstantBuffer, sizeof(PerObjectPBR));
	auto cbPerObjPBRPtr = cbPerObjPBR.map_as<PerObjectPBR>();
	spinAngle = fmodf(spinAngle + 0.1f*3.14159f*dt, 2 * 3.14159);
	cbPerObjPBRPtr->modelMatrix = glm::scale(glm::rotate(glm::mat4(1.0f), spinAngle, glm::vec3{ 0, 1, 0 }), glm::vec3(0.01f));
	cbPerObjPBRPtr->objectColor = glm::vec4(1.0f);
	cbPerObjPBRPtr->eta = 1.40f;

	auto &cbEnvmap = Renderer::allocTransientBuffer(BufferUsage::ConstantBuffer, sizeof(EnvCubeParams));
	auto cbEnvmapPtr = cbEnvmap.map_as<EnvCubeParams>();
	cbEnvmapPtr->modelMatrix =
		glm::translate(
			glm::scale(
			glm::translate(glm::vec3(context.sceneData.eyePos.x, context.sceneData.eyePos.y, context.sceneData.eyePos.z)),
				glm::vec3{ 1000.0f, 1000.0f, 1000.0f }),
			glm::vec3{ -0.5f, -0.5f, -0.5f });

	opaqueList.setRenderTarget(*screenRT2);
	float color[] = { 0.25f, 0.25f, 0.2f, 0.0f };
	opaqueList.clearColor(color);
	opaqueList.clearDepth(1.0f);

	opaqueList.setShader(shaderEnvCube.get());
	opaqueList.setConstantBuffers({ context.sceneDataCB, &cbEnvmap });
	opaqueList.setTextures({ envmap }, { Renderer::getSampler_LinearClamp() });
	mesh->draw(opaqueList, 0);

	//skel_anim_sampler->nextFrame();
	//auto pose = skel_anim_sampler->getPose(*skel, glm::mat4(1.0));
	//skel_debug->drawSkeleton(*skel, *skel_anim_sampler, context);
	//mokou_skin->update(pose);

	for (auto submesh = 0u; submesh < mokou->submeshes.size(); ++submesh)
	{
		opaqueList.setShader(shaderPBR.get());
		opaqueList.setConstantBuffers({ context.sceneDataCB, &cbPerObjPBR });
		opaqueList.setTextures(
			{ tex, envmap }, 
			{ Renderer::getSampler_LinearClamp(), Renderer::getSampler_LinearClamp() });
		mokou->draw(opaqueList, submesh);
	}

	// render terrain
	//terrain->render(context);

	// PostFX pass
	auto &cbFxParams = Renderer::allocTransientBuffer(BufferUsage::ConstantBuffer, sizeof(FXParams));
	auto cbFxParamsPtr = cbFxParams.map_as<FXParams>();
	cbFxParamsPtr->thing = 2.0;
	cbFxParamsPtr->vx_offset = 1.0;
	cbFxParamsPtr->rt_w = 1280;
	cbFxParamsPtr->rt_h = 720;

	CommandBuffer postProc;
	postProc.setScreenRenderTarget();
	float screenColor[] = { 0.2, 0.7, 0.2, 1.0 };
	postProc.clearColor(screenColor);
	postProc.clearDepth(1.0f);

	postProc.setShader(passthrough.get());
	postProc.setTextures(
		{ &screenRT2->getColorTexture(0), &screenRT2->getDepthTexture() },
		{ Renderer::getSampler_LinearClamp(), Renderer::getSampler_LinearClamp() });
	postProc.setConstantBuffers({ &cbFxParams });
	postProc.drawProcedural(PrimitiveType::Triangle, 0, 3, 0, 1);

	Logging::screenMessage("F       : " + std::to_string(numFrames));
	Logging::screenMessage("DT      : " + std::to_string(dt));
	Logging::screenMessage("E       : " + std::to_string(world.size()));

	Renderer::execute(opaqueList);
	Renderer::execute(postProc);
	renderDebugHud();
}

void RiftGame::renderDebugHud()
{
	CommandBuffer cmdBuf;
	auto lines = Logging::clearScreenMessages();
	unsigned ypos = 5;
	unsigned xpos = 5;
	unsigned yinc = debugFont->getMetrics().height;
	for (auto &&line : lines)
	{
		// drop shadow FTW
		hud->render(
			cmdBuf,
			line,
			*debugFont,
			glm::vec2(xpos+2, ypos+2),
			glm::vec2(sizeX, sizeY),
			Color::Black,
			glm::vec4(0.0, 0.0, 0.0, 0.0));
		hud->render(
			cmdBuf,
			line,
			*debugFont,
			glm::vec2(xpos, ypos),
			glm::vec2(sizeX, sizeY),
			Color::White,
			glm::vec4(0.0, 0.0, 0.0, 0.0));
		ypos += yinc;
	}
	Renderer::execute(cmdBuf);
	nvgBeginFrame(nvg_context, sizeX, sizeY, 1.0f);
	nvgBeginPath(nvg_context);
	nvgRect(nvg_context, 100, 100, 120, 30);
	nvgCircle(nvg_context, 120, 120, 5);
	nvgPathWinding(nvg_context, NVG_HOLE);   // Mark circle as a hole.
	nvgFillColor(nvg_context, nvgRGBA(255, 192, 0, 255));
	nvgFill(nvg_context);
	nvgEndFrame(nvg_context);
}

void RiftGame::update(float dt)
{
	auto win_size = Engine::instance().getWindow().size();
	sizeX = win_size.x;
	sizeY = win_size.y;

	mLastTime += dt;
	mTotalTime += dt;

	// mise à jour du compteur FPS toutes les secondes
	if (mLastTime > 1.f) {
		mLastTime = 0.f;
		mFPS = 1.f / dt;
		Engine::instance().getWindow().setTitle(("Rift (" + std::to_string(mFPS) + " FPS)").c_str());
	}

	++numFrames;
}

void RiftGame::tearDown()
{
	TwDeleteAllBars();
}

int main()
{
	Window window("Rift", 1280, 720);
	Engine::run<RiftGame>(window);
	return 0;
}
