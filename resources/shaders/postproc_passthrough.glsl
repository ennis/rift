#version 430
layout (binding = 0) uniform sampler2D colorTex;
layout (binding = 1) uniform sampler2D depthTex;

#pragma include <../postproc/vertex.glsl>

#ifdef _FRAGMENT_
in vec4 screenPos;
out vec4 oColor;

void main()
{
	oColor = texture(colorTex, screenPos.xy);
}

#endif