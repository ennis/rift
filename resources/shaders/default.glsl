// Default material shader, no lighting, diffuse texturing only
#version 430

// test include
#pragma include <scene.glsl>

layout(std140, binding = 2) uniform PerObject {
	mat4 modelMatrix;
};

layout (binding = 0) uniform sampler2D diffuseMap;

//=============================================================
// La macro _VERTEX_ est définie par le code qui va charger le shader (classe Effect)
#ifdef _VERTEX_

// postion: 3 floats, index 0 (c'est le premier attribut)
layout(location = 0) in vec3 position;
// normals: 3 floats
layout(location = 1) in vec3 normal;
// tg: 3 floats
layout(location = 2) in vec3 tangent;
// texcoords: 2 floats
layout(location = 3) in vec2 uv;


//--- OUT ----------------------------
// variables en sortie du vertex shader
out vec3 wPos;
out vec3 wN;
out vec3 vPos;
out vec2 tex;

//--- CODE ---------------------------
void main() 
{
	vec4 modelPos = modelMatrix * vec4(position, 1.f);
	gl_Position = viewProjMatrix * modelPos;
	wPos = modelPos.xyz;
	vPos = (viewMatrix * modelPos).xyz;
	// TODO normalmatrix
	wN = (modelMatrix * vec4(normal, 0.f)).xyz;
	wN = (modelMatrix * vec4(tangent, 0.f)).xyz;
	tex = uv;
}

#endif

//=============================================================
#ifdef _FRAGMENT_ 

//--- IN -----------------------------
// variables d'entrée
in vec3 wPos;
in vec3 wN;
in vec3 vPos;
in vec2 tex;

//--- OUT ----------------------------
// variables de sortie
out vec4 oColor;

const vec4 vertColor = vec4(0.9f, 0.9f, 0.1f, 1.0f);	

void main()
{
	vec2 tex2 = vec2(tex.x, 1.0f-tex.y);
	oColor = texture(diffuseMap, tex2.xy);
}

#endif