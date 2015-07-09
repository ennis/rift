#version 330

//--- IN -----------------------------
layout(location = 0) in vec3 position;
layout(location = 2) in vec4 color;

//--- OUT ----------------------------
out vec4 fcolor;

//--- UNIFORMS -----------------------
layout(std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;
	vec4 eyePos;	// in world space
	vec4 lightDir;
	vec2 viewportSize;
} rd;

uniform mat4 modelMatrix;

//--- CODE ---------------------------
void main() 
{
	gl_Position = rd.viewProjMatrix * modelMatrix * vec4(position, 1.f);
	fcolor = color;
}
