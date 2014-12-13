#version 430
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
out vec4 fColor;
layout(binding = 0, std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;
	vec4 eyePos;	// in world space
	vec4 lightDir;
	vec2 viewportSize;
} gRenderData;
uniform mat4 modelMatrix;
void main()
{
	gl_Position = gRenderData.viewProjMatrix * modelMatrix * vec4(position.xyz,1);
	fColor = color;
}