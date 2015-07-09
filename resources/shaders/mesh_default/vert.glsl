#version 430

//--- IN -----------------------------
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 texcoord;

//--- OUT ----------------------------
out vec3 fPosition;
out vec3 fNormal;
out vec3 fTangent;
out vec3 fBitangent;
out vec2 fTexcoord;

//--- UNIFORMS -----------------------
layout(binding = 0, std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;
	vec4 eyePos;	// in world space
	vec4 lightDir;
	vec2 viewportSize;
} gRenderData;

uniform mat4 modelMatrix;

//--- CODE ---------------------------
void main() 
{
	vec4 modelPos = modelMatrix * vec4(position, 1.f);
	gl_Position = gRenderData.viewProjMatrix * modelPos;
	fPosition = modelPos.xyz;
	fNormal = normal;
	fTangent = tangent;
	fBitangent = bitangent;
	fTexcoord = texcoord;
}
