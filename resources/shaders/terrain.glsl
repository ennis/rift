#version 430

#pragma include <scene.glsl>

layout(binding = 0) uniform sampler2D heightmap;
layout(binding = 1) uniform sampler2D normalMap;
layout(binding = 2) uniform sampler2D slopeTex;
layout(binding = 3) uniform sampler2D flatTex;

// constant
layout(std140, binding = 1) uniform TerrainUniforms
{
	mat4 modelMatrix;
	// size of the heightmap in pixels
	vec2 heightmapSize;
	// heightmap vertical scale in meters/unit
	float heightmapScale;
	// texture scale
	float flatTextureScale;
	float slopeTextureScale;
}

// per-patch update
layout(std140, binding = 2) uniform TerrainPatchUniforms
{
	// position of the grid in world space (in meters)
	vec2 patchOffset;
	// size of the grid (in meters, scale)
	float patchScale;
	// LOD level
	int lodLevel;
}


//===================================================================
#ifdef _VERTEX_

//--- IN -----------------------------
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 normals;

//--- OUT ----------------------------
out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexcoord;
out float altitude;

//--- CODE ---------------------------
float sampleHeight(vec2 coords)
{
	vec2 nc = coords / heightmapSize;
	return heightmapScale*texture(heightmap, nc).x;
}

void main() 
{
	const float ddp = 1.0;
	const vec2 dx = vec2(ddp, 0);
	const vec2 dy = vec2(0, ddp);

	// model space 2D position (heightmap sampling coordinates)
	// position in [0,1]
	vec2 pos = patchOffset + position * patchScale;

	//    h0
	// h1 hi h2
	//    h3
	/*float h0 = sampleHeight(pos-dy); 
	float h1 = sampleHeight(pos-dx); 
	float h2 = sampleHeight(pos+dx); 
	float h3 = sampleHeight(pos+dy); 
	float hp = sampleHeight(pos);

	float d0 = abs(h0-hp), d1 = abs(h1-hp), d2=abs(h2-hp), d3=abs(h3-hp);
	float ds = d0+d1+d2+d3+0.01;
	d0 /= ds; d1 /= ds; d2 /= ds; d3 /= ds; 
	vec2 dir = 0.5 * (d0*(-dy) + d1*(-dx) + d2*(+dx) + d3*(+dy)); */

	vec2 dd = vec2(sampleHeight(pos+dx)-sampleHeight(pos-dx),
					sampleHeight(pos+dy)-sampleHeight(pos-dy));
	vec3 normal = normalize(vec3(-dd.x, 2.0*ddp, -dd.y));
	//vec3 normal = texture(normalMap, ((pos+vec2(0.5,0.5))/heightmapSize)).xyz;

	// model space 3D position
	vec4 disp = vec4(pos.x, sampleHeight(pos), pos.y, 1.f);
    // position in clip space
    gl_Position = gRenderData.viewProjMatrix * modelMatrix * disp;

    // world-space position
    fPosition = (modelMatrix * disp).xyz;
    fNormal = normal;
    //fNormal = vec3(hp,hp,hp)/heightmapScale;
	fTexcoord = pos / heightmapSize;
	altitude = disp.y;
}

#endif

//===================================================================
#ifdef _FRAGMENT_

//--- IN -----------------------------
in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexcoord;
in float altitude;

//--- OUT ----------------------------
out vec4 oColor;


//--- CODE ---------------------------
float sampleHeight2(vec2 tex)
{
	return heightmapScale*texture(heightmap, tex).x;
}

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
void main()
{	
	//vec4 tmpcolor = lodColorMap[lodLevel];
	//vec4 tmpcolor = vec4(0.7, 0.7, 0.7, 1.0);

	float flatness = smoothstep(0.8, 0.85, abs(fNormal.y));
	float dir = abs(atan(fNormal.z, fNormal.x) / 3.14159);
	float snow = smoothstep(80, 90, altitude);

	vec4 flatColor = texture(flatTex, fPosition.xz);
	vec4 slopeColorX = texture(slopeTex, fPosition.yz);
	vec4 slopeColorZ = texture(slopeTex, fPosition.xy);

	vec4 final = mix(
					mix(
						mix(slopeColorX, slopeColorZ, dir), 
						flatColor, flatness),
					vec4(0.95f, 0.95f, 0.95f, 1.0f), snow);

	oColor = PhongIllum(final, 
				fNormal, // normal
				fPosition,	// position
				0.0,	// ka
				0.4,	// ks
				0.8, 	// kd
				1, 
				1, 
				1);
	//oColor = vec4(slope, slope, slope, 1.0f);
}

#endif