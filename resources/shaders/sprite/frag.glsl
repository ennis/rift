#version 430

//--- IN -----------------------------
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

layout(binding = 0) uniform sampler2D tex0;

//--- CODE ---------------------------
void main()
{	
	color = texture2D(tex0, fTexcoord);
}