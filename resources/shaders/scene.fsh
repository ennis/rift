
#ifdef SCENE_DEFERRED
// TODO
#else
out vec4 color;
#endif

struct Fragment {
	vec4 color;
	vec3 position;
	vec3 normal;
	vec3 emission;
};


layout(binding = 0, std140) uniform SceneParameters {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;
	vec4 eyePos;	// in world space
	vec4 lightDir;	// in world space
	vec2 viewportSize;
} gRenderData;

void SceneFog(inout Fragment frag) {
	// TODO fog
}

void SceneLighting_Lambertian(inout Fragment frag)
{
	// TODO light source intensity
	frag.color.xyz *= max(0.0, dot(gRenderData.lightDir.xyz, frag.normal));
}

// forward decl
void pixelShader(inout Fragment frag);

void main()
{
	Fragment f;
	pixelShader(f);
	color = f.color;
}