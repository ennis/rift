#pragma include <../postproc/params.glsl>

#ifdef _VERTEX_
out vec4 screenPos;
void main()
{
	// merci pascal
	float x = -1.0 + float((gl_VertexID & 1) << 2);
	float y = -1.0 + float((gl_VertexID & 2) << 1);
	vec2 tex = 0.5 * (vec2(x, y) + vec2(1.0, 1.0));
	gl_Position = vec4(x, y, 0, 1);
	screenPos.xy = tex;
	screenPos.zw = tex * viewportSize;
}
#endif
