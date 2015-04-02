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

class SkeletonDebug
{
public:
	static const unsigned kMaxJoints = 60;

	SkeletonDebug()
	{
		shader = gl4::Effect::loadFromFile("resources/shaders/skeleton_debug.glsl")->compileShader();
		cylinder = generateCylinderMesh();
		// Max 60 bones
		cb_bone_transforms = ConstantBuffer::create(kMaxJoints * sizeof(glm::mat4), nullptr);
		paramBlock = ParameterBlock::create(*shader);
	}

	void drawSkeleton(
		const Skeleton &skeleton,
		const SkeletonAnimationSampler &animSampler,
		RenderTarget2 &renderTarget,
		const SceneData &sceneData,
		const ConstantBuffer &sceneDataCB)
	{
		assert(skeleton.joints.size() < kMaxJoints);
		// compute pose
		auto pose = animSampler.getPose(skeleton, glm::mat4(1.0));
		for (size_t i = 0; i < pose.size(); i++)
		{
			pose[i] = pose[i] * glm::scale(glm::vec3(0.01*skeleton.joints[i].init_offset.length()));
		}

		// upload list of transforms
		cb_bone_transforms->update(0, pose.size() * sizeof(glm::mat4), pose.data());
		// draw instanced (num joints)
		paramBlock->setConstantBuffer(0, sceneDataCB);
		paramBlock->setConstantBuffer(1, *cb_bone_transforms);
		renderTarget.getRenderQueue().drawInstanced(*cylinder, 0, *shader, *paramBlock, pose.size(), 0);
	}

private:
	Shader::Ptr shader;
	Mesh::Ptr cylinder;
	ParameterBlock::Ptr paramBlock;
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
	unsigned long numFrames;

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

	SceneData sceneData;

	Entity *cameraEntity;
	Mesh::Ptr mesh;
	Mesh::Ptr mokou;

	gl4::Effect::Ptr effect;
	gl4::Effect::Ptr effectEnvCube;
	gl4::Effect::Ptr effectPBR;

	Shader::Ptr shader;
	Shader::Ptr shaderWireframe;
	Shader::Ptr shaderPBR;
	Shader::Ptr shaderEnvCube;

	ParameterBlock::Ptr paramBlock;
	ParameterBlock::Ptr paramBlockPBR;
	ParameterBlock::Ptr paramBlockEnvCube;

	ConstantBuffer::Ptr cbSceneData;
	ConstantBuffer::Ptr cbPerObj;
	ConstantBuffer::Ptr cbPerObjPBR;
	ConstantBuffer::Ptr cbEnvCube;

	Texture2D::Ptr tex;
	TextureCubeMap::Ptr envmap;
	
	Texture2D::Ptr shadowMap;
	RenderTarget::Ptr shadowRT;

	RenderTarget2::Ptr screenRT2;

	Shader::Ptr passthrough;

	struct FXParams
	{
		float thing;
		float vx_offset;
		float rt_w;
		float rt_h;
	};

	ConstantBuffer::Ptr fxCB;
	ParameterBlock::Ptr fxPB;

	Font::Ptr font;
	std::unique_ptr<HUDTextRenderer> hud;

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

	util::small_vector<int, 16> ints;

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
		{ ElementFormat::Float3, ResourceUsage::Static },
		{ ElementFormat::Float3, ResourceUsage::Static },
		{ ElementFormat::Float2, ResourceUsage::Static },
	};

	Submesh sm { PrimitiveType::Triangle, 0, 0, 8, 36 };

	mesh = Mesh::create(
		attribs,
		8,
		cubeMeshData,
		36,
		cubeIndices,
		{ sm },
		ResourceUsage::Static);

	std::ifstream mokou_file("resources/models/animated/mokou.mesh", std::ios::binary);
	serialization::IArchive arc(mokou_file);
	mokou = Mesh::loadFromArchive(arc);

	tex = Image::loadFromFile("resources/img/brick_wall.jpg").convertToTexture2D();
	envmap = Image::loadFromFile("resources/img/env/uffizi/env.dds").convertToTextureCubeMap();

	cbSceneData = ConstantBuffer::create(sizeof(SceneData), nullptr);
	cbPerObj = ConstantBuffer::create(sizeof(PerObject), nullptr);
	cbPerObjPBR = ConstantBuffer::create(sizeof(PerObjectPBR), nullptr);
	cbEnvCube = ConstantBuffer::create(sizeof(EnvCubeParams), nullptr);

	paramBlock = ParameterBlock::create(*shader);
	paramBlockPBR = ParameterBlock::create(*shaderPBR);
	paramBlockEnvCube = ParameterBlock::create(*shaderEnvCube);

	glm::ivec2 win_size = Engine::instance().getWindow().size();
	shadowMap = Texture2D::create(win_size, 1, ElementFormat::Depth16, nullptr);
	shadowRT = RenderTarget::createRenderTarget2D(*shadowMap, 0);

	font = Font::loadFromFile("resources/img/fonts/arno_pro.fnt");
	hud = std::make_unique<HUDTextRenderer>();

	DepthStencilDesc ds_fx;
	ds_fx.depthTestEnable = false;
	ds_fx.depthWriteEnable = false;
	passthrough = gl4::Effect::loadFromFile("resources/shaders/fxaa.glsl")->compileShader({}, RasterizerDesc{}, ds_fx, BlendDesc{});
	screenRT2 = RenderTarget2::create({ 1280, 720 }, { ElementFormat::Unorm8x4 }, ElementFormat::Depth24);

	fxCB = ConstantBuffer::create(sizeof(FXParams), nullptr);
	fxPB = ParameterBlock::create(*passthrough);
	fxPB->setConstantBuffer(0, *fxCB);

	fxPB->setTextureParameter(0, &screenRT2->getColorTexture(0), SamplerDesc{});
	fxPB->setTextureParameter(1, &screenRT2->getDepthTexture(), SamplerDesc{});

	std::vector<BVHMapping> mappings;
	std::ifstream bvh("resources/models/bvh/man_skeleton.bvh");
	skel = Skeleton::loadFromBVH(bvh, mappings);

	std::ifstream motion_file("resources/models/bvh/man_walk.bvh");
	skel_debug = std::make_unique<SkeletonDebug>();
	skel_animation = SkeletonAnimation::loadFromBVH(motion_file, *skel, mappings);
	skel_anim_sampler = std::make_unique<SkeletonAnimationSampler>(*skel, skel_animation, 0.003f);
	skel_anim_sampler->nextFrame();
}


void RiftGame::render(float dt)
{
	// rendu de la scene
	glm::ivec2 win_size = Engine::instance().getWindow().size();
	auto &R = Renderer::getInstance();

	screenRT2->clearColor(0.25f, 0.25f, 0.2f, 0.0f);
	screenRT2->clearDepth(1.0f);
	auto &&rq = screenRT2->getRenderQueue();
	//R.setViewports({ { 0.f, 0.f, float(win_size.x), float(win_size.y), 0.0f, 1.0f } });

	// update scene data buffer
	auto cam = cameraEntity->getComponent<Camera>();
	sceneData.eyePos = glm::vec4(cameraEntity->getTransform().position, 1.0f);
	sceneData.projMatrix = cam->getProjectionMatrix();
	sceneData.viewMatrix = cam->getViewMatrix();
	sceneData.viewProjMatrix = sceneData.projMatrix * sceneData.viewMatrix;
	sceneData.viewportSize = win_size;
	cbSceneData->update(0, sizeof(SceneData), &sceneData);

	// update per-model buffer
	perObj.modelMatrix = glm::mat4(1.0f);
	perObj.objectColor = glm::vec4(1.0f);
	cbPerObj->update(0, sizeof(PerObject), &perObj);

	// update per-model buffer
	spinAngle = fmodf(spinAngle + 0.1f*3.14159f*dt, 2 * 3.14159);
	perObjPBR.modelMatrix = glm::rotate(glm::mat4(1.0f), spinAngle, glm::vec3{ 0, 1, 0 });
	perObjPBR.objectColor = glm::vec4(1.0f);
	perObjPBR.eta = 1.40f;
	cbPerObjPBR->update(0, sizeof(PerObjectPBR), &perObjPBR);

	envCubeParams.modelMatrix = 
		glm::translate(
			glm::scale(
				glm::translate(glm::vec3(sceneData.eyePos.x, sceneData.eyePos.y, sceneData.eyePos.z)),
				glm::vec3{ 1000.0f, 1000.0f, 1000.0f }),
			glm::vec3{ -0.5f, -0.5f, -0.5f });
	cbEnvCube->update(0, sizeof(EnvCubeParams), &envCubeParams);

	// create parameter block
	paramBlock->setConstantBuffer(0, *cbSceneData);
	paramBlock->setConstantBuffer(1, *cbPerObj);
	paramBlock->setTextureParameter(0, tex.get(), SamplerDesc{});

	paramBlockPBR->setConstantBuffer(0, *cbSceneData);
	paramBlockPBR->setConstantBuffer(1, *cbPerObjPBR);
	paramBlockPBR->setTextureParameter(0, tex.get(), SamplerDesc{});
	paramBlockPBR->setTextureParameter(1, envmap.get(), SamplerDesc{});

	paramBlockEnvCube->setConstantBuffer(0, *cbSceneData);
	paramBlockEnvCube->setConstantBuffer(1, *cbEnvCube);
	paramBlockEnvCube->setTextureParameter(0, envmap.get(), SamplerDesc{});

	// submit to render queue
	if (glfwGetKey(Engine::instance().getWindow().getHandle(), GLFW_KEY_H)) {
		//renderQueue.debugPrint();
	}

	if (glfwGetKey(Engine::instance().getWindow().getHandle(), GLFW_KEY_W)) {
		// XXX not the same parameter block!
		//renderQueue->draw(*mesh, 0, *shaderWireframe, *paramBlock, 0);
	}
	else {
		//renderQueue->draw(*mesh, 0, *shader, *paramBlock, 0);
	}

	skel_anim_sampler->nextFrame();
	skel_debug->drawSkeleton(*skel, *skel_anim_sampler, *screenRT2, sceneData, *cbSceneData);
	rq.draw(*mesh, 0, *shaderEnvCube, *paramBlockEnvCube, 0);

	for (auto submesh = 0u; submesh < mokou->getNumSubmeshes(); ++submesh)
		rq.draw(*mokou, submesh, *shaderPBR, *paramBlockPBR, 0);

	//sky.render(*renderQueue, sceneData, *cbSceneData);
	screenRT2->flush();

	// PostFX pass
	auto &screen_rt = RenderTarget2::getDefaultRenderTarget();
	auto &screen_rq = screen_rt.getRenderQueue();
	FXParams fxp;
	fxp.thing = 2.0;
	fxp.vx_offset = 1.0;
	fxp.rt_w = 1280;
	fxp.rt_h = 720;
	fxCB->update(0, sizeof(FXParams), &fxp);
	screen_rq.drawProcedural(PrimitiveType::Triangle, 3, *passthrough, *fxPB, 0);
	hud->renderString("Hello world!", *font, { 100.0, 100.0 }, Color::White, Color::Black, screen_rq, sceneData, *cbSceneData);
	screen_rt.flush();

	// render tweak bar
	//TwDraw();
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
