#version 430

#pragma include <scene.glsl>

layout(std140, binding = 1) uniform CBSkyParams {
	vec3 sunDirection;
	vec3 skyColor;
};

//=============================================================
#ifdef _VERTEX_

layout(location = 0) in vec3 position;
out vec3 fPosition;
void main() {
	vec4 modelPos = vec4(position, 1.f);
    gl_Position = viewProjMatrix * modelPos;
    fPosition =  modelPos.xyz;
}

#endif

//=============================================================
#ifdef _FRAGMENT_ 

in vec3 fPosition;
out vec4 oColor;
const vec3 cSkyColor = vec3(
	0.18867780436772762, 
	0.4978442963618773, 
	0.6616065586417131);
void main() 
{
	// eye ray in world space
	vec3 ray = normalize(fPosition - wEye.xyz);
	float ext = pow(max(0, dot(ray, -sunDirection)), 2);
	float scatter = smoothstep(0.1, 0.3, (1.f - sunDirection.y) / 2);
	vec3 color = skyColor * ext + cSkyColor * scatter;
	oColor = vec4(color, 1.f);
}

#endif