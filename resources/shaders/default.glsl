// version GLSL 4.30 parce que pourquoi pas.
#version 430

// test include
#pragma include <scene.glsl>

// tous les paramètres doivent être placés dans des uniform buffers
layout(std140, binding = 1) uniform PerObject {
	mat4 modelMatrix;
	vec4 color;
};

// sauf les paramètres de texture
layout (binding = 0) uniform sampler2D diffuse;

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
	gl_Position = viewProjMatrix * modelPos;
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
	oColor = texture(diffuse, fPosition.xy);
}

#endif