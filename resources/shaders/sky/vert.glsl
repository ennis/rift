#version 430

//--- IN -----------------------------
layout(location = 0) in vec3 position;

//--- OUT ----------------------------
out vec3 fPosition;

//--- UNIFORMS -----------------------
layout(binding = 0, std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;
	vec4 eyePos;	// in world space
	vec4 lightDir;
	vec2 viewportSize;
} gRenderData;

//--- CODE ---------------------------
void main() {
	vec4 modelPos = vec4(position, 1.f);
    gl_Position = gRenderData.viewProjMatrix * modelPos;
    fPosition =  modelPos.xyz;
}