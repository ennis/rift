// version GLSL 4.30 parce que pourquoi pas.
#version 430	
// Ceci est le vertex shader utilisé dans test_mesh

//--- IN -----------------------------
// On spécifie les attributs en entrée du vertex shader
// L'index indiqué pour 'location' doit correspondre à la position de l'attribut 
// dans le tableau passé en paramètre à Mesh::allocate

// postion: 3 floats, index 0 (c'est le premier attribut)
layout(location = 0) in vec3 position;
// normals: 3 floats
layout(location = 1) in vec3 normal;
// texcoords: 2 floats
layout(location = 2) in vec2 texcoord;

//--- OUT ----------------------------
// variables en sortie du vertex shader
out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexcoord;

//--- UNIFORMS -----------------------
// Ici on déclare le buffer contenant les paramètres partagés
// Il est attaché au slot 0 (binding=0): il correspond donc à la structure
// 'PerFrameShaderParameters' définie dans test_mesh/main.cpp
// !!! Les données doivent être agencées en mémoire de la même façon 
// dans le fichier .cpp et dans le shader : i.e. les types doivent correspondre
// et les variables doivent être définies dans le même ordre 
layout(binding = 0, std140) uniform RenderData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;	// = projMatrix*viewMatrix
	vec4 eyePos;	// in world space
	vec4 lightDir;
	vec2 viewportSize;	// taille de la fenêtre
} gRenderData;

// model matrix 
uniform mat4 modelMatrix;

//--- CODE ---------------------------
void main() 
{
	vec4 modelPos = modelMatrix * vec4(position, 1.f);
	gl_Position = gRenderData.viewProjMatrix * modelPos;
	fPosition = modelPos.xyz;
	fNormal = normal;
	fTexcoord = texcoord;
}
