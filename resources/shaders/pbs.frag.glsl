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

@interface vec3 finalColor(vec4 color, vec3 normal, vec3 bump);

uniform samplerCube gEnvmap;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecular;
// roughness
// normal map

const float PI = 3.141596;
const float shininess = 10000.f;	// Blinn-Phong shininess

vec3 fresnel(vec3 specularColor, vec3 L, vec3 H)
{
	// Schlick's approximation
	return specularColor + (1.f - specularColor) * pow(max(0, (1 - dot(L, H))), 5.0);
}

float blinnPhongDistribution(vec3 N, vec3 H)
{
	return (shininess + 2.0f) / (2 * PI) * pow(max(0, dot(N, H)), shininess);
}

vec3 microfacetBRDF(vec3 F0, vec2 texcoord, vec3 L, vec3 H, vec3 N, float NdotL)
{
	// Microfacet BRDF:
	// fspecular = F(L, H) * G(L, V, H) * D(H) / (4 * NdotL * NdotV)
	// F(L, H): fresnel term
	// G(L, V, H): geometry term
	// D(H): distribution term

	// F(L, H)
	vec3 vfresnel = fresnel(F0, L, H);

	// D(H) Blinn-Phong distribution
	float fblinnphong = blinnPhongDistribution(N, H);
	
	// Geometry term: implicit (NdotL * NdotV)
	vec3 specular = fblinnphong * vfresnel * NdotL;
	return specular;
}


//--- CODE ---------------------------
void main()
{
	vec3 L = normalize(gRenderData.lightDir.xyz - fposition);
	vec3 N = normalize(fnormal);
	vec3 V = normalize(gRenderData.eyePos.xyz - fposition);
	vec3 H = normalize(V + L);
	vec3 R = reflect(-V, N);
	vec3 HR = normalize(V + R);

	float NdotL = max(0, dot(N, L));
	float NdotR = max(0, dot(N, R));
	// diffuse term
	vec3 diffuse = texture(gDiffuse, ftexcoord).rgb * NdotL;
	// approx diffuse from envmap

	// Specular from light source 
	vec3 F0 = vec3(0.5, 0.5, 0.1);
	vec3 specular = microfacetBRDF(F0, ftexcoord, L, H, N, NdotL);

	// Specular from envmap 
	vec3 specular_env = microfacetBRDF(F0, ftexcoord, R, HR, N, NdotR);
	
	//color = vec4(0.001f * specular_env * texture(gEnvmap, R).rgb, 1.f);
	
	color = vec4(diffuse, 1.f);
}