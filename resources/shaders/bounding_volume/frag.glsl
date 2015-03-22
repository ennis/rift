#version 430
// Ceci est le fragment shader utilisé pour bouding_volume

//--- IN -----------------------------
in vec3 fPosition;

//--- OUT ----------------------------
out vec4 oColor;

//--- UNIFORMS -----------------------
layout(binding = 0, std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;	// = projMatrix*viewMatrix
	vec4 eyePos;	// in world space
	vec4 lightDir;
	vec2 viewportSize;	// taille de la fenêtre
} gRenderData;

// paramètres
uniform float lightIntensity;
uniform vec4 volumeColor;

//--- CODE ---------------------------
void main()
{
	oColor = lightIntensity * volumeColor;
}