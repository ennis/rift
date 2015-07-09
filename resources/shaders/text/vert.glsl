#version 430

layout(std140, binding=0) uniform TextParams
{
	mat4 transform;
	vec4 fillColor;
	vec4 outlineColor;
};

layout(binding = 0) uniform sampler2D fontTex;

#ifdef _VERTEX_
layout(location=0) in vec4 postex;
out vec2 tex;
void main()
{
	gl_Position=transform*vec4(postex.x,postex.y,0,1);
	 /*vec4((position.x/uViewSize.x)*2-1,(1-position.y/uViewSize.y)*2-1,0, 1);*/
	tex=postex.zw;
}
#endif

#ifdef _FRAGMENT_
in vec2 tex;
out vec4 omColor;
void main()
{
	float v = texture(fontTex, tex).r;
	float blend = v > 0.5 ? (2*v-1) : 0;
	vec4 color = mix(outlineColor, fillColor, blend);
	color.a *= v > 0.5 ? 1 : 2*v;
	omColor = color;
}
#endif