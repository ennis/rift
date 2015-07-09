#version 430

//--- IN -----------------------------
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 normals;

//--- OUT ----------------------------
out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexcoord;

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
uniform ivec2 heightmapSize;
// heightmap vertical scale in meters/unit
uniform float heightmapScale;
// LOD level
uniform int lodLevel;

//--- CODE ---------------------------
float sampleHeight(vec2 coords)
{
	vec2 nc = coords / heightmapSize;
	return heightmapScale*texture(heightmap, nc).x;
}

void main() 
{
	const float ddp = 1.0;
	const vec2 dx = vec2(1.0, 0);
	const vec2 dy = vec2(0, 1.0);

	// model space 2D position (heightmap sampling coordinates)
	// position in [0,1]
	vec2 pos = patchOffset + position * patchScale;

	vec2 dd = vec2(sampleHeight(pos+dx)-sampleHeight(pos-dx),
					sampleHeight(pos+dy)-sampleHeight(pos-dy));

	vec3 normal = normalize(vec3(-dd.x, 2.0*ddp, -dd.y));

	// model space 3D position
	vec4 disp = vec4(pos.x, sampleHeight(pos), pos.y, 1.f);
    // position in clip space
    gl_Position = gRenderData.viewProjMatrix * modelMatrix * disp;

    // world-space position
    fPosition = (modelMatrix * disp).xyz;
    fNormal = normal;
	fTexcoord = pos / heightmapSize;
}
