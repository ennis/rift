#version 430

//--- IN -----------------------------
in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexcoord;

//--- OUT ----------------------------
out vec4 color;

//--- UNIFORMS -----------------------
layout(binding = 0, std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;
	vec4 eyePos;	// in world space
	vec4 lightDir;	// in world space
	vec2 viewportSize;
} gRenderData;

layout(binding = 0) uniform sampler2D heightmap;
uniform mat4 modelMatrix;
// position of the grid in world space (in meters)
uniform vec2 patchOffset;
// size of the grid (in meters, scale)
uniform float patchScale;
// size of the heightmap in pixels
uniform vec2 heightmapSize;
// heightmap vertical scale in meters/unit
uniform float heightmapScale;
// LOD level
uniform int lodLevel;

//--- CODE ---------------------------
const vec4 lodColorMap[10] = vec4[](
	vec4(1.0, 0.0, 0.0, 1.0),
	vec4(0.9, 0.1, 0.0, 1.0),
	vec4(0.8, 0.2, 0.0, 1.0),
	vec4(0.7, 0.3, 0.0, 1.0),
	vec4(0.6, 0.4, 0.0, 1.0),
	vec4(0.5, 0.5, 0.0, 1.0),
	vec4(0.4, 0.6, 0.0, 1.0),
	vec4(0.3, 0.7, 0.0, 1.0),
	vec4(0.2, 0.8, 0.0, 1.0),
	vec4(0.1, 0.9, 0.0, 1.0)
);

void main()
{	
	color = lodColorMap[lodLevel];
}