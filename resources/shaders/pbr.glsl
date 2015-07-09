// version GLSL 4.30 parce que pourquoi pas.
#version 430

#pragma include <scene.glsl>

layout(std140, binding = 1) uniform PerObject {
	mat4 modelMatrix;
	vec4 color;
	float eta;
};

layout (binding = 0) uniform sampler2D diffuse;
layout (binding = 1) uniform samplerCube envmap;

//=============================================================
#ifdef _VERTEX_

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

out vec3 wPos;
out vec3 wN;
out vec3 vPos;
out vec2 tex;

void main() 
{
	vec4 modelPos = modelMatrix * vec4(position, 1.f);
	gl_Position = viewProjMatrix * modelPos;
	wPos = modelPos.xyz;
	vPos = (viewMatrix * modelPos).xyz;
	// TODO normalmatrix
	wN = (modelMatrix * vec4(normal, 0.f)).xyz;
	tex = texcoord;
}

#endif

//=============================================================
#ifdef _FRAGMENT_ 

in vec3 wPos;
in vec3 wN;
in vec3 vPos;
in vec2 tex;

out vec4 omColor;

const vec4 vertColor = vec4(0.9f, 0.9f, 0.1f, 1.0f);	

// direction in world space
vec4 sampleEnvmap(vec3 wDn)
{
	return texture(envmap, wDn);
}

/*float fresnel(float eta, float cosTheta)
{
    float R0 = (1.0-eta)*(1.0-eta) / ((1.0+eta)*(1.0+eta));
    float m = 1.0 - cosTheta;
    return R0 + (1.0 - R0)*m*m*m*m*m;
}*/

void main( void )
{
	vec3 wNn = normalize(wN);
	vec3 wVn = normalize(wEye.xyz - wPos);
	vec4 Creflected = sampleEnvmap(reflect(-wVn, wNn));
	vec3 wRrn = refract(-wVn, wNn, 1.0/eta);
	vec4 Crefracted = sampleEnvmap(wRrn);

	float F = fresnel(eta, max(0.0, dot(wNn, wVn)));
	if (wRrn != vec3(0.0))
		omColor = mix(Crefracted, Creflected, F);
	else
		// non transparent or total internal reflection
		omColor = Creflected * F;
}

#endif