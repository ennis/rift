#version 430

//--- IN -----------------------------
in vec3 fPosition;

//--- OUT ----------------------------
out vec4 oColor;

//--- UNIFORMS -----------------------
layout(binding = 0, std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;
	vec4 eyePos;	// in world space
	vec4 lightDir;	// in world space
	vec2 viewportSize;
} gRenderData;

uniform vec3 uSunColor;
uniform vec3 uSunDirection;

//--- CODE ---------------------------
const vec3 skyColor = vec3(
	0.18867780436772762, 
	0.4978442963618773, 
	0.6616065586417131);

void main() 
{
	// eye ray in world space
	vec3 ray = normalize(fPosition - gRenderData.eyePos.xyz);
	float ext = pow(max(0, dot(ray, -uSunDirection)), 2);
	float scatter = smoothstep(0.1, 0.3, (1.f - uSunDirection.y) / 2);
	vec3 color = uSunColor * ext + skyColor * scatter;
	oColor = vec4(color, 1.f);
}

