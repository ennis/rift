#version 330

//--- IN -----------------------------
in vec3 fposition;
in vec3 fnormal;
in vec2 ftexcoord;

//--- OUT ----------------------------
out vec4 color;

//--- UNIFORMS -----------------------
layout(std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;
	vec4 eyePos;	// in world space
	vec4 lightDir;	// in world space
	vec2 viewportSize;
} gRenderData;

uniform samplerCube gEnvmap;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecular;
// roughness
// normal map

const float PI = 3.141596;
const float shininess = 5.f;	// Blinn-Phong shininess

vec3 fresnel(vec3 specularColor, vec3 L, vec3 H)
{
	// Schlick's approximation
	return specularColor + (1 - specularColor) * pow(1 - dot(L, H), 5.0);
}

float blinnPhongDistribution(vec3 N, vec3 H)
{
	return (shininess + 2) / (2 * PI) * pow(dot(N, H), shininess);
}


//--- CODE ---------------------------
void main()
{
	vec3 L = gRenderData.lightDir.xyz;
	vec3 N = fnormal;
	vec3 V = normalize(gRenderData.eyePos.xyz - fposition);
	vec3 H = (V + N) / 2;
	vec3 R = reflect(N, V);

	float NdotL = dot(N, L);
	// diffuse term
	vec3 diffuse = texture(gDiffuse, ftexcoord).rgb * NdotL;

	// Microfacet BRDF:
	// fspecular = F(L, H) * G(L, V, H) * D(H) / (4 * NdotL * NdotV)
	// F(L, H): fresnel term
	// G(L, V, H): geometry term
	// D(H): distribution term

	// F(L, H)
	vec3 vfresnel = fresnel(texture(gSpecular, ftexcoord).rgb, L, H);

	// D(H) Blinn-Phong distribution
	float fblinnphong = blinnPhongDistribution(N, H);

	// Geometry term: implicit (NdotL * NdotV)
	vec3 specular = vfresnel * fblinnphong * NdotL * 0.25 * texture(gEnvmap, R).rgb;
	
	color = vec4(diffuse + specular, 1.f);
}