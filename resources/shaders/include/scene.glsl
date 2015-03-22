// Test include GLSL

// Ici on déclare le buffer contenant les paramètres partagés
// Il est attaché au slot 0 (binding=0): il correspond donc à la structure
// 'PerFrameShaderParameters' définie dans test_mesh/main.cpp
// !!! Les données doivent être agencées en mémoire de la même façon 
// dans le fichier .cpp et dans le shader : i.e. les types doivent correspondre
// et les variables doivent être définies dans le même ordre 
layout(std140, binding = 0) uniform SceneData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;	// = projMatrix*viewMatrix
	vec4 wEye;	// in world space
	vec4 lightDir;
	vec2 viewportSize;	// taille de la fenêtre
};


/*float fresnel(float eta, float cosTheta)
{
    float R0 = (1.0-eta)*(1.0-eta) / ((1.0+eta)*(1.0+eta));
    float m = 1.0 - cosTheta;
    return R0 + (1.0 - R0)*m*m*m*m*m;
}

vec4 PhongIllum(
	vec4 albedo, 
	vec3 normal, 
	vec3 position,
	float ka,
	float ks,
	float kd, 
	float lightIntensity, 
	float eta, 
	float shininess)
{
	vec4 Ln = normalize(gRenderData.lightDir),
         Nn = normalize(vec4(normal, 0.0)),
         Vn = normalize(gRenderData.eyePos - vec4(position, 1.0f));
    vec4 H = normalize(Ln + Vn);
    // Ambient
    vec4 ambient = ka * lightIntensity * albedo;
    // Diffuse
    vec4 diffuse = kd * max(dot(Nn, Ln), 0.0) * albedo;
    // Specular
    vec4 Rn = reflect(-Ln, Nn);
    vec4 specular = ks * albedo * pow(max(dot(Rn, Vn), 0.0), shininess) * lightIntensity;
    specular *= fresnel(eta, dot(H, Vn));
	return vec4((ambient + diffuse + specular).xyz, 1.0);
	// TEST
	//return vec4(position, 1.0f);
}*/