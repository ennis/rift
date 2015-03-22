// version GLSL 4.30 parce que pourquoi pas.
#version 430	
// Ceci est le vertex shader utilisé pour bouding_volume

//--- IN -----------------------------

// postion: 3 floats, index 0 (c'est le premier attribut)
layout(location = 0) in vec3 position;

//--- OUT ----------------------------
out vec3 fPosition;

//--- UNIFORMS -----------------------
layout(binding = 0, std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;	// = projMatrix*viewMatrix
	vec4 eyePos;	// in world space
	vec4 lightDir;
	vec2 viewportSize;	// taille de la fenêtre
} gRenderData;

// model matrix 
uniform mat4 modelMatrix;

//--- CODE ---------------------------
void main() 
{
	vec4 modelPos = modelMatrix * vec4(position, 1.f);
	gl_Position = gRenderData.viewProjMatrix * modelPos;
	fPosition = modelPos.xyz;
}