// shader for immediate mode, in wireframe
#version 430

layout(std140, binding = 0) uniform ImmediateParams 
{
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;
	mat4 modelMatrix;
	vec4 wEye;	// in world space
	vec4 lineColor;
	vec2 viewportSize;
};

//=============================================================
#ifdef _VERTEX_
#ifdef FULL_MESH_ATTRIBS
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;
#else
layout(location = 0) in vec3 position;
#endif
out vec3 wPos;
void main() 
{
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
	oColor = lineColor;
}
#endif
