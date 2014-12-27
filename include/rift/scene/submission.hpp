#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP

#include <renderer.hpp>
#include <material.hpp>
#include <transform.hpp>


//
// Une 'Submission' est une commande de rendu que l'on passe au Renderer par l'intermédiaire d'une RenderQueue.
// Ces commandes ne sont pas exécutées immédiatement. Elles sont stockées dans des listes (les RenderQueues)
// puis triées pour optimiser le rendu (par exemple: tri par shader, par profondeur, etc.)
// Ces objets contiennent tous les états nécessaires pour lancer une commande de rendu, à savoir:
//	- l'effect (les shaders)
//  - les paramètres (matériaux)
//  - la transformation (model->world)
//  - les vertex/index buffers à binder
//  - le vertex layout
//  - la submesh (start index & vertex)
//  - le type de primitives (tri, lines, tristrip, etc.)
//  - ...
struct Submission
{
	// default-constructible, moveable, copyable, etc...

	//-------------------
	// transform
	Transform transform;
	// TODO world bounding box

	//-------------------
	// viewport & bucket
	int viewportID;
	int bucketID;

	//-------------------
	// high-level material state
	// (effect + effect parameters)
	const Material *material;

	//-------------------
	// vertex data input
	std::array<VertexBuffer*, 16> VB;
	IndexBuffer *IB;
	VertexLayout *vertexLayout;
	
	//-------------------
	// draw command
	PrimitiveType primitiveType;
	int instanceCount;
	int startVertex;
	int numVertices;
	int startIndex;
	int numIndices;
};

#endif