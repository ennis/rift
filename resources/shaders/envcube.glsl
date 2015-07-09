#version 430
#pragma include <scene.glsl>

layout(std140, binding = 1) uniform PerObject {
	mat4 modelMatrix;
};

layout (binding = 0) uniform samplerCube envmap;

//=============================================================
#ifdef _VERTEX_

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;
out vec3 wPos;

void main() 
{
	// center on eye
	vec4 modelPos = modelMatrix * vec4(position, 1.f);
	gl_Position = viewProjMatrix * modelPos;
	wPos = modelPos.xyz;
}

#endif

//=============================================================
#ifdef _FRAGMENT_ 

in vec3 wPos;
out vec4 oColor;

void main()
{
	vec3 wVn = normalize(wPos - wEye.xyz);
	oColor = texture(envmap, wVn);
}

#endif