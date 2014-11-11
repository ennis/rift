#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP

#include <renderer.hpp>
#include <effect.hpp>

struct Submission
{
	//-------------------
	// viewport & bucket
	int viewportID;
	int bucketID;
	
	//-------------------
	// high-level material state
	// => effect
	Effect *effect;

	// Things needed to create a pipeline state:
	// * Current pass ID
	// * Material state:
	//     - VS
	//     - Tesselation
	//     - PS (texture+lighting)
	//     - Render state (blending, depth test, depth write)
	// * Global engine state:
	//     - Fog/Atmosphere shader
	//     - Render state override 
	// * Shader parameters (per-instance)
	//     - Constant buffer/range
	//     - Per-instance data (EffectParameters)
	// 
	// To create a pipeline state:
	// * Normal pass: Combine VS / (TCS/TES) / PS / Atmosphere shader into one program, cache the result
	//		- GLSL: combine PS+Atmosphere shader into one shader source 
	// 

	// TODO TCS, TES, geometry shaders


	//-------------------
	// vertex data input
	VertexBuffer *VB;
	IndexBuffer *IB;
	VertexLayout *vertexLayout;
	
	//-------------------
	// draw command
	PrimitiveType primitiveType;
	int instanceCount;
	int vertexStartOffset;
	int vertexCount;
	int indexStartOffset;
	int indexCount;
};

#endif