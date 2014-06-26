#version 330

in vec4 fcolor;
out vec4 out_color;

layout(std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;
	vec3 lightDir;
	ivec2 viewportSize;
} rd;

void main()
{
	out_color = fcolor;
}