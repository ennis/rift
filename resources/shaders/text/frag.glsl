#version 430
uniform vec4 uFillColor;
uniform vec4 uOutlineColor;
layout(binding = 0) uniform sampler2D fontTex;
in vec2 fTexcoord;
out vec4 oColor;
void main()
{
	float v = texture(fontTex, fTexcoord).r;
	float blend = v > 0.5 ? (2*v-1) : 0;
	vec4 color = mix(uOutlineColor, uFillColor, blend);
	color.a *= v > 0.5 ? 1 : 2*v;
	oColor = color;
}