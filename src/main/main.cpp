#include <game.hpp>
#include <string>
#include <transform.hpp>
#include <log.hpp>
#include <freecameracontrol.hpp>
#include <entity.hpp>
#include <AntTweakBar.h>
#include <serialization.hpp>
#include <animationclip.hpp>
#include <scene.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <image.hpp>
#include <sky.hpp>
#include <gl4/renderer.hpp>
#include <gl4/effect.hpp>	// Effect
#include <font.hpp>
#include <hudtext.hpp>
#include <colors.hpp>
#include <small_vector.hpp>
#include <skeleton.hpp>
#include <animationcurve.hpp>
#include <skeletonanimation.hpp>
#include <resources.hpp>
#include <mesh.hpp>
#include <terrain.hpp>

// utils
namespace
{
	Mesh::Ptr generateCylinderMesh()
	{
		// HA!
		std::ifstream fileIn("resources/models/sphere.mesh", std::ios::binary);
		serialization::IArchive arc(fileIn);
		return Mesh::loadFromArchive(arc);
	}

	/*glm::mat4 match2points(glm::vec3 s0, glm::vec3 s1, glm::vec3 t0, glm::vec3 t1)
	{
		// estimate scale
		auto ds = s1 - s0;
		auto dt = t1 - t0;
		float scaling = dt.length() / ds.length();
	}*/
}

class SkinnedMesh
{
public:
	SkinnedMesh(Resources &resources, const char *path, const char *invBindPosesPath, const Skeleton &skel) :
		mesh(resources.meshes.load(path)),
		skeleton(skel)
	{
		std::ifstream fileIn(invBindPosesPath);
		serialization::IArchive arc(fileIn);
		// load inverse bind poses
		for (auto i = 0u; i < skeleton.joints.size(); ++i) {

		}
	}

private:
	const Skeleton &skeleton;
	Mesh *mesh;
	std::vector<glm::mat4> invBindPoses;
};

class SkeletonDebug
{
public:
	static const unsigned kMaxJoints = 60;

	SkeletonDebug()
	{
		shader = gl4::Effect::loadFromFile("resources/shaders/skeleton_debug.glsl")->compileShader();
		cylinder = generateCylinderMesh();
		// Max 60 bones
		//cb_bone_transforms = ConstantBuffer::create(kMaxJoints * sizeof(glm::mat4), nullptr);
	}

	void drawSkeleton(
		const Skeleton &skeleton,
		const SkeletonAnimationSampler &animSampler,
		RenderTarget2 &renderTarget,
		const SceneData &sceneData)
	{
		assert(skeleton.joints.size() < kMaxJoints);
		// compute pose
		auto pose = animSampler.getPose(skeleton, glm::mat4(1.0));
		for (size_t i = 0; i < pose.size(); i++)
		{
			pose[i] = pose[i] * glm::scale(glm::vec3(0.01*skeleton.joints[i].init_offset.length()));
		}

		// upload list of transforms
		//cb_bone_transforms->update(0, pose.size() * sizeof(glm::mat4), pose.data());
		// draw instanced (num joints)
		//paramBlock->setConstantBuffer(0, sceneDataCB);
		//paramBlock->setConstantBuffer(1, *cb_bone_transforms);
		//renderTarget.getRenderQueue().drawInstanced(*cylinder, 0, *shader, *paramBlock, pose.size(), 0);

		// VS
		/*for (...)
		{
			cmdBuf.initRenderCommand();
			params[i] = pose[i];
			cmdBuf.setBuffers({BufferDesc(params, ptr, size)})
			mesh.draw(cmdBuf);
		}*/
	}

private:
	Shader::Ptr shader;
	Mesh::Ptr cylinder;
	ConstantBuffer::Ptr cb_bone_transforms;
};

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

private:
	float mLastTime = 0.f;
	float mTotalTime = 0.f;
	float mFPS = 0;
	float spinAngle = 0.f;
	unsigned long numFrames = 0;

	std::unique_ptr<Resources> resources;

	struct PerObject
	{
		glm::mat4 modelMatrix;
		glm::vec4 objectColor;
	} perObj;

	struct PerObjectPBR
	{
		glm::mat4 modelMatrix;
		glm::vec4 objectColor;
		float eta;
	} perObjPBR;

	struct EnvCubeParams
	{
		glm::mat4 modelMatrix;
	} envCubeParams;

	Entity *cameraEntity;
	Mesh::Ptr mesh;
	Mesh *mokou = nullptr;

	gl4::Effect::Ptr effect;
	gl4::Effect::Ptr effectEnvCube;
	gl4::Effect::Ptr effectPBR;

	Shader::Ptr shader;
	Shader::Ptr shaderWireframe;
	Shader::Ptr shaderPBR;
	Shader::Ptr shaderEnvCube;

	Stream::Ptr cbSceneData;
	Stream::Ptr cbPerObj;
	Stream::Ptr cbPerObjPBR;
	Stream::Ptr cbEnvmap;
	Stream::Ptr cbFxParams;

	Texture2D *tex;
	TextureCubeMap *envmap;
	
	RenderTarget2::Ptr shadowRT;
	RenderTarget2::Ptr screenRT2;

	Shader::Ptr passthrough;

	struct FXParams
	{
		float thing;
		float vx_offset;
		float rt_w;
		float rt_h;
	};

	Font::Ptr font;
	std::unique_ptr<HUDTextRenderer> hud;
	std::unique_ptr<Terrain> terrain;

	Sky sky;
	std::unique_ptr<Skeleton> skel;
	std::unique_ptr<SkeletonDebug>  skel_debug;
	SkeletonAnimation skel_animation;
	std::unique_ptr<SkeletonAnimationSampler> skel_anim_sampler;
};

//============================================================================
void RiftGame::init()
{
	boost::filesystem::path path(".");

	resources = std::make_unique<Resources>();

	// create the camera object
	
	// create the camera entity (maybe not necessary...)
	cameraEntity = Entity::create();
	// create a camera object
	auto camera = cameraEntity->addComponent<Camera>();
	// center camera position
	cameraEntity->getTransform().move(glm::vec3(0, 0, -1));
	// add camera controller script
	cameraEntity->addComponent<FreeCameraController>();

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

	/*std::ifstream mokou_file("resources/models/animated/mokou.mesh", std::ios::binary);
	serialization::IArchive arc(mokou_file);
	mokou = Mesh::loadFromArchive(arc);*/
	mokou = resources->meshes.load("resources/models/rock/crystal.mesh");
	tex = resources->textures.load("resources/img/brick_wall.jpg");
	envmap = resources->cubeMaps.load("resources/img/env/uffizi/env.dds");
	envmap = resources->cubeMaps.load("resources/img/env/uffizi/env.dds");	// TEST

	cbSceneData = Stream::create(BufferUsage::ConstantBuffer, sizeof(SceneData), 3);
	cbPerObj = Stream::create(BufferUsage::ConstantBuffer, sizeof(PerObject), 3);
	cbPerObjPBR = Stream::create(BufferUsage::ConstantBuffer, sizeof(PerObjectPBR), 3);
	cbEnvmap = Stream::create(BufferUsage::ConstantBuffer, sizeof(EnvCubeParams), 3);
	cbFxParams = Stream::create(BufferUsage::ConstantBuffer, sizeof(FXParams), 3);

	glm::ivec2 win_size = Engine::instance().getWindow().size();
	shadowRT = RenderTarget2::create(win_size, {}, ElementFormat::Depth16);

	font = Font::loadFromFile("resources/img/fonts/arno_pro.fnt");
	hud = std::make_unique<HUDTextRenderer>();

	DepthStencilDesc ds_fx;
	ds_fx.depthTestEnable = false;
	ds_fx.depthWriteEnable = false;
	passthrough = gl4::Effect::loadFromFile("resources/shaders/fxaa.glsl")->compileShader({}, RasterizerDesc{}, ds_fx, BlendDesc{});
	screenRT2 = RenderTarget2::create({ 1280, 720 }, { ElementFormat::Unorm8x4 }, ElementFormat::Depth24);

	std::vector<BVHMapping> mappings;
	std::ifstream bvh("resources/models/bvh/man_skeleton.bvh");
	skel = Skeleton::loadFromBVH(bvh, mappings);

	std::ifstream motion_file("resources/models/bvh/man_walk.bvh");
	skel_debug = std::make_unique<SkeletonDebug>();
	skel_animation = SkeletonAnimation::loadFromBVH(motion_file, *skel, mappings);
	skel_anim_sampler = std::make_unique<SkeletonAnimationSampler>(*skel, skel_animation, 0.003f);
	skel_anim_sampler->nextFrame();

	terrain = std::make_unique<Terrain>(
		Image::loadFromFile("resources/img/terrain/test_heightmap_2.dds"),
		resources->textures.load("resources/img/mb_rocklface07_d.dds"),
		resources->textures.load("resources/img/grasstile_c.dds"));
}


void RiftGame::render(float dt)
{
	// rendu de la scene
	glm::ivec2 win_size = Engine::instance().getWindow().size();
	glm::vec2 win_size_f = glm::vec2(win_size.x, win_size.y);
	auto &R = Renderer::getInstance();

	RenderQueue2 opaqueRenderQueue;
	RenderQueue2 overlayRenderQueue;
	SceneRenderContext context;
	context.opaqueRenderQueue = &opaqueRenderQueue;
	context.overlayRenderQueue = &overlayRenderQueue;
	context.textRenderer = hud.get();
	context.defaultFont = font.get();

	// update scene data buffer
	auto cam = cameraEntity->getComponent<Camera>();
	context.sceneData.eyePos = glm::vec4(cameraEntity->getTransform().position, 1.0f);
	context.sceneData.projMatrix = cam->getProjectionMatrix();
	context.sceneData.viewMatrix = cam->getViewMatrix();
	context.sceneData.viewProjMatrix = context.sceneData.projMatrix * context.sceneData.viewMatrix;
	context.sceneData.viewportSize = win_size;
	context.sceneData.lightDir = glm::vec4(0.f, 1.f, 0.f, 0.f);
	cbSceneData->write(context.sceneData);
	context.sceneDataCB = cbSceneData->getDescriptor();	

	// update per-model buffer
	perObj.modelMatrix = glm::mat4(1.0f);
	perObj.objectColor = glm::vec4(1.0f);
	cbPerObj->write(perObj);

	// update per-model buffer
	spinAngle = fmodf(spinAngle + 0.1f*3.14159f*dt, 2 * 3.14159);
	perObjPBR.modelMatrix = glm::rotate(glm::mat4(1.0f), spinAngle, glm::vec3{ 0, 1, 0 });
	perObjPBR.objectColor = glm::vec4(1.0f);
	perObjPBR.eta = 1.40f;
	cbPerObjPBR->write(perObjPBR);

	envCubeParams.modelMatrix = 
		glm::translate(
			glm::scale(
			glm::translate(glm::vec3(context.sceneData.eyePos.x, context.sceneData.eyePos.y, context.sceneData.eyePos.z)),
				glm::vec3{ 1000.0f, 1000.0f, 1000.0f }),
			glm::vec3{ -0.5f, -0.5f, -0.5f });
	cbEnvmap->write(envCubeParams);

	//skel_anim_sampler->nextFrame();
	//skel_debug->drawSkeleton(*skel, *skel_anim_sampler, *screenRT2, sceneData, *cbSceneData);

	opaqueRenderQueue.beginCommand();
	opaqueRenderQueue.setShader(*shaderEnvCube);
	opaqueRenderQueue.setUniformBuffers({ context.sceneDataCB, cbEnvmap->getDescriptor() });
	opaqueRenderQueue.setTextureCubeMap(0, *envmap, SamplerDesc{});
	//mesh->draw(opaqueRenderQueue, 0);

	for (auto submesh = 0u; submesh < mesh->submeshes.size(); ++submesh)
	{
		opaqueRenderQueue.beginCommand();
		opaqueRenderQueue.setShader(*shaderPBR);
		opaqueRenderQueue.setUniformBuffers({ context.sceneDataCB, cbPerObjPBR->getDescriptor() });
		opaqueRenderQueue.setTexture2D(0, *tex, SamplerDesc{});
		opaqueRenderQueue.setTextureCubeMap(1, *envmap, SamplerDesc{});
		//mokou->draw(opaqueRenderQueue, submesh);
	}

	// render terrain
	terrain->render(context);

	// fence all constant buffer streams
	cbEnvmap->fence(opaqueRenderQueue);
	cbSceneData->fence(opaqueRenderQueue);
	cbPerObj->fence(opaqueRenderQueue);
	cbPerObjPBR->fence(opaqueRenderQueue);

	screenRT2->clearColor(0.25f, 0.25f, 0.2f, 0.0f);
	screenRT2->clearDepth(1.0f);
	screenRT2->commit(opaqueRenderQueue);

	// PostFX pass
	auto &screen_rt = RenderTarget2::getDefaultRenderTarget();
	FXParams fxp;
	fxp.thing = 2.0;
	fxp.vx_offset = 1.0;
	fxp.rt_w = 1280;
	fxp.rt_h = 720;
	cbFxParams->write(fxp);
	RenderQueue2 tmp;
	tmp.beginCommand();
	tmp.setShader(*passthrough);
	tmp.setTexture2D(0, screenRT2->getColorTexture(0), SamplerDesc{});
	tmp.setTexture2D(0, screenRT2->getDepthTexture(), SamplerDesc{});
	tmp.setUniformBuffers({ cbFxParams->getDescriptor() });
	tmp.draw(PrimitiveType::Triangle, 0, 3, 0, 1);
	screen_rt.clearColor(0.2, 0.7, 0.2, 1.0);
	screen_rt.clearDepth(1.0);
	screen_rt.commit(tmp);
	hud->renderText(*context.overlayRenderQueue, "Hello world!", *font, { 10.0, 10.0 }, win_size_f, Color::White, Color::Black);
	hud->renderText(*context.overlayRenderQueue, "Frame " + std::to_string(numFrames), *font, { 10.0, 50.0 }, win_size_f, Color::White, Color::Black);
	hud->renderText(*context.overlayRenderQueue, "dt " + std::to_string(dt), *font, { 10.0, 90.0 }, win_size_f, Color::White, Color::Black);
	hud->fence(*context.overlayRenderQueue);
	cbFxParams->fence(overlayRenderQueue);
	screen_rt.commit(overlayRenderQueue);
}

void RiftGame::update(float dt)
{
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
	Entity::destroy(cameraEntity);
}

int main()
{
	logInit("log");
	Window window("Rift", 1280, 720);
	Engine::run<RiftGame>(window);
	return 0;
}
