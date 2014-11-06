#version 430
uniform mat4 uTransform;
layout(location=0) in vec4 postex;
out vec2 fTexcoord;
void main()
{
	gl_Position=uTransform*vec4(postex.x,postex.y,0,1);
	 /*vec4((position.x/uViewSize.x)*2-1,(1-position.y/uViewSize.y)*2-1,0, 1);*/
	fTexcoord=postex.zw;
}