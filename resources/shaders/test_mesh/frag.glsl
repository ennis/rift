#version 430
// Ceci est le fragment shader utilisé dans test_mesh

//--- IN -----------------------------
// variables d'entrée
in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexcoord;

//--- OUT ----------------------------
// variables de sortie
out vec4 oColor;

//--- UNIFORMS -----------------------
layout(binding = 0, std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;	// = projMatrix*viewMatrix
	vec4 eyePos;	// in world space
	vec4 lightDir;
	vec2 viewportSize;	// taille de la fenêtre
} gRenderData;
// paramètre eta
uniform float shininess;
uniform float eta;
uniform float lightIntensity;

const vec4 vertColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);	 

//--- CODE ---------------------------
// (copié du TP OpenGL)
// NB: dans l'exemple, les normales sont fausses 
float fresnel(float eta, float cosTheta)
{
    float R0 = (1.0-eta)*(1.0-eta) / ((1.0+eta)*(1.0+eta));
    float m = 1.0 - cosTheta;
    return R0 + (1.0 - R0)*m*m*m*m*m;
}

void main()
{
	/*vec4 Ln = normalize(-gRenderData.lightDir),
         Nn = normalize(vec4(fNormal, 0.0)),
         Vn = normalize(gRenderData.eyePos - vec4(fPosition, 1.0f));
    vec4 H = normalize(Ln + Vn);
    // Ambient
    float ka = 0.2;
    vec4 ambient = ka * lightIntensity * vertColor;
    // Diffuse
    float kd = 0.3;
    vec4 diffuse = kd * max(dot(Nn, Ln), 0.0) * vertColor;
    // Specular
    float ks = 0.4;
    vec4 Rn = reflect(-Ln, Nn);
    vec4 specular = ks * vertColor * pow(max(dot(Rn, Vn), 0.0), shininess) * lightIntensity;
    specular *= fresnel(eta, dot(H, Vn));
	oColor = ambient + diffuse + specular;*/

	oColor = vec4(fPosition, 1.0f);
}
