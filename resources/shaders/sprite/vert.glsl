#version 430

//--- IN -----------------------------
layout(location = 0) in vec2 position;

//--- OUT ----------------------------
out vec2 fTexcoord;

//--- UNIFORMS -----------------------
layout(binding = 0, std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;
	vec4 eyePos;	// in world space
	vec4 lightDir;
	vec2 viewportSize;
} gRenderData;

uniform ivec2 offset;
uniform ivec2 size;

//--- CODE ---------------------------
void main() 
{
	vec2 pos = (offset + position.xy * size) * 2 - 1;
    gl_Position = vec4(pos.x, pos.y, 0, 1);
    fTexcoord = position.xy;
}
