#version 330

//--- IN -----------------------------
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

//--- OUT ----------------------------
out vec3 fnormal;
out vec2 ftexcoord;

//--- UNIFORMS -----------------------
layout(std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;
	vec4 lightDir;
	vec2 viewportSize;
} rd;

uniform mat4 modelMatrix;

//--- CODE ---------------------------
void main() 
{
	gl_Position = rd.viewProjMatrix * modelMatrix * vec4(position, 1.f);
	fnormal = normal;
}
