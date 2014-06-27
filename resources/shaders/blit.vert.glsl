#version 330

//--- IN -----------------------------
layout(location = 0) in vec3 position;

//--- OUT ----------------------------
out vec2 ftexcoord;

//--- UNIFORMS -----------------------
layout(std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;
	vec4 eyePos;	// in world space
	vec4 lightDir;
	vec2 viewportSize;
} rd;

uniform vec2 screenPosition;
uniform vec2 textureSize;

//--- CODE ---------------------------
void main() {
	vec2 pos = (screenPosition + position.xy * textureSize / rd.viewportSize) * 2 - 1;
    gl_Position = vec4(pos.x, pos.y, 0, 1);
    ftexcoord = position.xy;
}