// version GLSL 4.30 parce que pourquoi pas.
#version 430

// test include
#pragma include <scene.glsl>

// paramètres communs aux vertex et fragment shaders
// model matrix 
uniform mat4 modelMatrix;
uniform float shininess;
uniform float eta;
uniform float lightIntensity;

//=============================================================
// La macro _VERTEX_ est définie par le code qui va charger le shader (classe Effect)
#ifdef _VERTEX_

// postion: 3 floats, index 0 (c'est le premier attribut)
layout(location = 0) in vec3 position;
// normals: 3 floats
layout(location = 1) in vec3 normal;
// texcoords: 2 floats
layout(location = 2) in vec2 texcoord;

//--- OUT ----------------------------
// variables en sortie du vertex shader
out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexcoord;

//--- CODE ---------------------------
void main() 
{
	vec4 modelPos = modelMatrix * vec4(position, 1.f);
	gl_Position = gRenderData.viewProjMatrix * modelPos;
	fPosition = modelPos.xyz;
	fNormal = normal;
	fTexcoord = texcoord;
}

#endif

//=============================================================
#ifdef _FRAGMENT_ 

//--- IN -----------------------------
// variables d'entrée
in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexcoord;

//--- OUT ----------------------------
// variables de sortie
out vec4 oColor;

const vec4 vertColor = vec4(0.9f, 0.9f, 0.1f, 1.0f);	

void main()
{
	oColor = PhongIllum(vertColor, 
				fNormal, // normal
				fPosition,	// position
				0.2,	// ka
				0.3,	// ks
				0.4, 	// kd
				lightIntensity, 
				eta, 
				shininess);
}

#endif