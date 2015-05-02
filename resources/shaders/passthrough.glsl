#version 430

layout(std140, binding=0) uniform Params
{
	float thing;
};

layout(binding = 0) uniform sampler2D colorTex;
layout(binding = 1) uniform sampler2D depthTex;

#ifdef _VERTEX_
out vec2 tex;
void main()
{
	// merci pascal
	float x = -1.0 + float((gl_VertexID & 1) << 2);
	float y = -1.0 + float((gl_VertexID & 2) << 1);
	tex.x = (x+1.0)*0.5;
	tex.y = (y+1.0)*0.5;
	gl_Position = vec4(x, y, 0, 1);
}
#endif

#ifdef _FRAGMENT_
in vec2 tex;
out vec4 omColor;
void main()
{
	//omColor = vec4(tex.x, tex.y, 0, 1);
	omColor = texture(colorTex, tex);
}
#endif

