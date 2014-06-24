#version 330

layout(location = 0) in vec3 position;
layout(location = 2) in vec4 color;

out vec4 fcolor;

uniform mat4 modelMatrix;
uniform mat4 viewProjMatrix;

void main() 
{
	gl_Position = viewProjMatrix * modelMatrix * vec4(position, 1.f);
	fcolor = color;
}
