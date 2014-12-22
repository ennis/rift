#include <game.hpp>	// class Game
#include <transform.hpp>	// struct Transform
#include <log.hpp>
#include <freecameracontrol.hpp>	// component FreeCameraController
#include <entity.hpp>	// class Entity
#include <renderer.hpp>	// class Renderer
#include <shadersource.hpp>	// loadShaderSource
#include <mesh.hpp>		// class Mesh
#include <renderable.hpp>	// struct PerFrameShaderParameters

#include <string>

//============================================================================
class TestMesh : public Game
{
public:
	TestMesh()
	{
		init();
	}

	void init();
	void initMesh();
	void initShader();
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

	// Le maillage
	Mesh mesh;
	// Le shader que l'on va utiliser pour afficher le maillage
	Shader *shader;
};

//============================================================================
void TestMesh::init()
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

	initMesh();
	initShader();
}

void TestMesh::initMesh()
{
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

	// allocation du mesh!

	// la liste des attributs des vertices. 
	// Elle décrit l'agencement en mémoire des éléments de notre vertex buffer.
	Mesh::Attribute attribs[] = { 
		/* 1er attribut: position */ 
		{ 
			/* on n'utilise qu'un seul buffer, donc l'index du buffer est 0 */ 
			0, 
			/* l'attribut est composé de 3 floats (X,Y et Z) */ 
			ElementFormat::Float3 
		},
		/* 2: normales */
		{ 
			/* idem */ 
			0, 
			/* idem */ 
			ElementFormat::Float3 
		}, 
		/* 3: texcoords */
		{ 
			/* idem */ 
			0, 
			/* cette fois-ci, il n'y a que deux floats (X,Y)*/ 
			ElementFormat::Float2
		}, 
	};

	// La liste des vertex buffers que l'on va allouer
	// ici, on va en allouer qu'un seul
	Mesh::Buffer buffers[] = { 
		/* buffer 0 */
		{ /* Utilisation du buffer: Static signifie que le buffer sera mis à jour une seule fois
		   * et plus jamais modifié ensuite.
		   * S'il devait être modifié à chaque frame, on aurait utilisé ResourceUsage::Dynamic
		   */ 
			ResourceUsage::Static 
		} 
	};

	// La liste des pointeurs vers les données initiales des vertex buffers
	// ici on ne va allouer qu'un seul buffer, que l'on va initialiser avec
	// les données contenues dans le tableau cubeMeshData
	const void *init[] = { cubeMeshData };

	// allocation
	mesh.allocate(
		Engine::instance().getRenderer(),
		// on dessine des triangles
		PrimitiveType::Triangle, 
		// il y a trois attributs par vertex
		3,
		// on passe la liste des attributs 
		attribs, 
		// on alloue qu'un seul vertex buffer
		1, 
		// on passe la liste des vertex buffer
		buffers, 
		// on a 8 vertices
		8, 
		// on initialise les vertex buffers avec les données pointées dans init
		init, 
		// on a 36 indices (soit 12 triangles)
		36, 
		// le format des indices est uint16_t (16-bit short)
		ElementFormat::Uint16, 
		// on ne changera pas les indices
		ResourceUsage::Static, 
		// on initialise l'index buffer avec les données contenues dans cubeIndices
		cubeIndices);

	// Voilà, les données des vertex sont maintenant sur le GPU, prêt à être dessinées

}


void TestMesh::initShader()
{
	// on charge les sources des shaders
	std::string vertexShaderSource = loadShaderSource("resources/shaders/test_mesh/vert.glsl");
	std::string fragmentShaderSource = loadShaderSource("resources/shaders/test_mesh/frag.glsl");

	// on appelle le renderer pour qu'il créé le shader
	Renderer &r = Engine::instance().getRenderer();
	shader = r.createShader(
		/* source du vertex shader */
		vertexShaderSource.c_str(), 
		/* source du fragment shader */
		fragmentShaderSource.c_str());
}

void TestMesh::render(float dt)
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

	// Dessin du mesh

	// on commence par dire au renderer quel shader on veut utiliser
	R.setShader(shader);
	// on indique au renderer quels buffers de paramètres (constant buffers) le shader utilise
	// ici: le shader utilise un buffer qui est attaché au slot 0 
	R.setConstantBuffer(0, frameData);
	// on envoie les paramètres spécifiques au shader
	// indice de refraction eta
	R.setNamedConstantFloat("eta", 2.0f);	// désolé pour les noms à rallonge, faudra changer ça un jour
	R.setNamedConstantFloat("shininess", 5.0f);	
	R.setNamedConstantFloat("lightIntensity", 1.0f);
	R.setNamedConstantMatrix4("modelMatrix", Transform().toMatrix());	// matrice identité
	// enfin, on dit au mesh de se dessiner
	mesh.draw();
	// en interne, Mesh::draw va faire plusieurs choses:
	//	- dire à OpenGL quels vertex buffers utiliser
	//	- dire à OpenGL quel index buffer utiliser
	//  - passer le layout des vertex buffers à OpenGL (c'est moche)
	//  - envoyer la commande de rendu (glDrawElementsXXX(...))

}

void TestMesh::update(float dt)
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

void TestMesh::tearDown()
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
	// Création de la classe principale TestMesh
	// et démarrage de la boucle principale 
	Engine::run<TestMesh>(window);
	return 0;
}
