#include <game.hpp>	// class Game
#include <transform.hpp>	// struct Transform
#include <log.hpp>
#include <freecameracontrol.hpp>	// component FreeCameraController
#include <entity.hpp>	// class Entity
#include <renderer.hpp>	// class Renderer
#include <model.hpp>		// class Model
#include <material.hpp>		// class Material
#include <effect.hpp>		// class Effect
#include <renderable.hpp>	// struct PerFrameShaderParameters
#include <modelrenderer.hpp>	// class ModelRenderer

#include <string>

//============================================================================
class TestModel : public Game
{
public:
	TestModel()
	{
		init();
	}

	void init();
	void initModel();
	void initMaterials();
	void render(float dt);
	void update(float dt);
	void tearDown();

private:
	float mLastTime = 0.f;
	float mTotalTime = 0.f;
	float mFPS = 0;

	// La caméra (a un composant Camera attaché)
	Entity *cameraEntity;
	// Buffer contenant les paramètres communs à passer au shader 
	ConstantBuffer *frameData;

	// Pour afficher un modèle à l'écran, on a besoin de ces 4 objets:

	// Le modèle que l'on va charger depuis un fichier
	Model model;
	// L'Effect que l'on va utiliser (vertex shader + fragment shader)
	Effect effect;
	// Le material pour effectuer le rendu du model
	Material material;
	// Enfin, l'objet qui va effectuer le rendu
	ModelRenderer modelRenderer;
};

//============================================================================
void TestModel::init()
{	
	Renderer &rd = Engine::instance().getRenderer();
	
	// Création de la caméra 
	// XXX mieux vaut ignorer le truc avec les entités
	{
		cameraEntity = Entity::create();
		auto camera = cameraEntity->addComponent<Camera>();
		cameraEntity->getTransform().move(glm::vec3(0, 0, -1));
		cameraEntity->addComponent<FreeCameraController>();
	}

	// Création du buffer des paramètres à passer au shader
	// TODO mieux vaut ignorer ça pour le moment, ça devrait pas être fait dans le main
	frameData = rd.createConstantBuffer(
		sizeof(PerFrameShaderParameters), 
		ResourceUsage::Dynamic, 
		nullptr);

	initModel();
	initMaterials();
}

void TestModel::initModel()
{
	// on charge un modèle depuis un fichier
	model = Model::loadFromFile(Engine::instance().getRenderer(), "resources/models/crystal.model");
	// maintenant l'objet model contient les données du modèle en RAM (positions, normales, squelette, etc.)
	// Il faut maintentant créer l'objet Mesh correspondant (qui va résider sur la mémoire du GPU)
	// On fait ça avec model.optimize()
	model.optimize();
	// maintenant, model.getMesh() va retourner un objet valide: on peut effectuer le rendu
	// TODO Créer le Mesh en même temps que le chargement du fichier
}


void TestModel::initMaterials()
{
	// Ici, on utilise les classes 'Effect' et 'Material' qui vont gérer les shaders 
	// et leurs paramètres pour nous.
	// En gros, la classe Effect représente un couple vertex+fragment shader
	// la classe Material contient une référence vers un effect et un ensemble de paramètres
	// à passer au shader

	// chargement de l'effect
	effect.loadFromFile("resources/shaders/default.glsl");
	// on associe l'effect au material
	material.setEffect(effect);
	// on peut maintenant définir des paramètres à passer aux shaders
	material.setParameter("eta", 2.0f);
	material.setParameter("shininess", 5.0f);
	material.setParameter("lightIntensity", 1.0f);

	// pour dessiner un modèle à l'écran, il faut un objet ModelRenderer
	// Il combine une référence à un modèle et une liste de Materials à utiliser pour 
	// chaque sous-mesh

	// on associe le model au modelRenderer
	modelRenderer.setModel(model);
	// on définit le material pour le sous-mesh #0 (il n'y en a qu'un dans le model chargé)
	modelRenderer.setMaterial(0, material);

	// Tout est OK, on est prêt à dessiner le modèle à l'écran
}

void TestModel::render(float dt)
{
	// rendu de la scene
	Renderer &R = Engine::instance().getRenderer();

	// la ligne suivante indique au renderer que l'on dessine à l'écran
	// (et non pas vers une texture comme on pourrait le faire pour du shadow mapping)
	R.bindRenderTargets(0, nullptr, nullptr);
	// on met à jour la taille du viewport opengl (pour qu'il corresponde à la taille de la fenêtre)
	glm::ivec2 win_size = Engine::instance().getWindow().size();
	Viewport vp{ 0.f, 0.f, float(win_size.x), float(win_size.y) };
	R.setViewports(1, &vp);
	// on efface le color buffer
	R.clearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
	// on efface le depth buffer
	R.clearDepth(100.f);

	// Ici, on initialise le RenderContext: la structure est passée en paramètre 
	// des methodes 'draw'
	// Elle contient les infos nécessaires sur la scène pour effectuer un rendu
	// (notamment, la position de la caméra, les matrices du vue et de projection)
	RenderContext rc;
	auto cam = cameraEntity->getComponent<Camera>();
	rc.viewMatrix = cam->getViewMatrix();
	rc.projMatrix = cam->getProjectionMatrix();
	rc.renderer = &R;
	rc.camera = cam;
	rc.perFrameShaderParameters = frameData;
	// inutile
	rc.renderPass = RenderPass::Opaque;

	// Les lignes suivantes mettent à jour le buffer contenant les paramètres communs à passer
	// à tous les shaders.
	// Ces paramètres contiennent entre autres les matrices de vue et de projection, le centre de la camera
	// les paramètres de lumière, la taille de la fenêtre, etc.
	// L'interêt d'avoir un buffer est de pouvoir le partager entre différents shaders sans
	// avoir à tout reconfigurer quand on change de shader
	PerFrameShaderParameters pfsp;
	pfsp.eyePos = glm::vec4(cameraEntity->getTransform().position, 0.0f);
	pfsp.lightDir = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	pfsp.projMatrix = rc.projMatrix;
	pfsp.viewMatrix = rc.viewMatrix;
	pfsp.viewportSize = win_size;
	pfsp.viewProjMatrix = rc.projMatrix * rc.viewMatrix;
	// on demande au renderer de mettre à jour le buffer avec les nouveaux paramètres
	R.updateBuffer(frameData, 0, sizeof(PerFrameShaderParameters), &pfsp);
	rc.pfsp = pfsp;

	// Dessin du modèle
	// RenderContext + model transform (inclut la position de l'objet)
	modelRenderer.render(rc, Transform());
}

void TestModel::update(float dt)
{
	mLastTime += dt;
	mTotalTime += dt;

	// mise à jour du compteur FPS toutes les secondes
	if (mLastTime > 1.f) {
		mLastTime = 0.f;
		mFPS = 1.f / dt;
		Engine::instance().getWindow().setTitle(("Rift (" + std::to_string(mFPS) + " FPS)").c_str());
	}
}

void TestModel::tearDown()
{
	Entity::destroy(cameraEntity);
}

int main()
{
	// Initialise le système de logs
	// logInit prend en paramètre un nom de fichier dans lequel les logs seront copiés
	logInit("test_mesh");
	// fenêtre principale
	Window window("Rift", 1280, 720);
	// Création de la classe principale TestModel
	// et démarrage de la boucle principale 
	Engine::run<TestModel>(window);
	return 0;
}
