#version 430

#pragma include <scene.glsl>

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

//===================================================================
#ifdef _VERTEX_

//--- IN -----------------------------
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 normals;

//--- OUT ----------------------------
out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexcoord;

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

#endif

//===================================================================
#ifdef _FRAGMENT_

//--- IN -----------------------------
in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexcoord;

//--- OUT ----------------------------
out vec4 oColor;

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

//--- CODE ---------------------------
float sampleTex(vec2 texcoords)
{
	return texture(heightmap, texcoords).x;
}

void main()
{	
	vec4 tmpcolor = lodColorMap[lodLevel];
	const float ddp = 2.0;
	const vec2 dx = vec2(ddp, 0)/heightmapSize.x;
	const vec2 dy = vec2(0, ddp)/heightmapSize.y;

	// http://stackoverflow.com/questions/5281261/generating-a-normal-map-from-a-height-map
	float s[9];
	s[0] = sampleTex(fTexcoord-dx-dy);
	s[1] = sampleTex(fTexcoord   -dy);
	s[2] = sampleTex(fTexcoord+dx-dy);
	s[3] = sampleTex(fTexcoord-dx);
	s[4] = sampleTex(fTexcoord);
	s[5] = sampleTex(fTexcoord+dx);
	s[6] = sampleTex(fTexcoord-dx+dy);
	s[7] = sampleTex(fTexcoord   +dy);
	s[8] = sampleTex(fTexcoord+dx+dy);

	vec3 n;
	n.x = heightmapScale * -(s[2]-s[0]+2*(s[5]-s[3])+s[8]-s[6]);
	n.y = heightmapScale * -(s[6]-s[0]+2*(s[7]-s[1])+s[8]-s[2]);
	n.z = 1.0;
	n = normalize(n);

	oColor = PhongIllum(tmpcolor, 
				n, // normal
				fPosition,	// position
				0.0,	// ka
				0.0,	// ks
				1.0, 	// kd
				1, 
				1, 
				1);
	//oColor = vec4(fNormal, 1.0f);
}

#endif