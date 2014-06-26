#version 330

layout(location = 0) in vec3 position;
layout(location = 2) in vec4 color;

out vec4 fcolor;

layout(std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;
	vec3 lightDir;
	ivec2 viewportSize;
} rd;

uniform mat4 modelMatrix;

void main() 
{
	gl_Position = rd.viewProjMatrix * modelMatrix * vec4(position, 1.f);
	fcolor = color;
}
