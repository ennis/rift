#version 330

//--- IN -----------------------------
in vec2 ftexcoord;

//--- OUT ----------------------------
out vec4 color;

//--- UNIFORMS -----------------------
uniform sampler2D tex0;

layout(std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;
	vec3 lightDir;
	ivec2 viewportSize;
} rd;

//--- CODE ---------------------------
void main()
{
	color = texture2D(tex0, ftexcoord);
}


