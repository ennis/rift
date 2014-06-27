#version 330

//--- IN -----------------------------
in vec3 fnormal;
in vec2 ftexcoord;

//--- OUT ----------------------------
out vec4 color;

//--- UNIFORMS -----------------------
layout(std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;
	vec4 lightDir;	// in world space
	vec2 viewportSize;
} gRenderData;

uniform samplerCube gEnvmap;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecular;
// roughness
// normal map

//--- CODE ---------------------------
void main()
{
	float LdotN = dot(gRenderData.lightDir.xyz, fnormal);
	color = vec4(LdotN);
}