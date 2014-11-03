#version 430
uniform ivec2 uViewSize;
uniform mat3 uTransform = mat3(1.0);
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texcoord;
out vec2 fTexcoord;
void main()
{
	gl_Position = vec4(
		(position.x/uViewSize.x)*2-1,
		(1-position.y/uViewSize.y)*2-1,
		0, 1);
	fTexcoord=texcoord;
}