#version 330

//--- IN -----------------------------
in vec4 fcolor;

//--- OUT ----------------------------
out vec4 out_color;

//--- UNIFORMS -----------------------
layout(std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;
	vec4 eyePos;	// in world space
	vec4 lightDir;
	vec2 viewportSize;
} rd;

//--- CODE ---------------------------
void main()
{
	out_color = fcolor;
}