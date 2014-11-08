#version 430

//--- IN -----------------------------
in vec3 fPosition;
in vec3 fNormal;
in vec3 fTangent;
in vec3 fBitangent;
in vec2 fTexcoord;

#pragma include <scene.fsh>

//--- CODE ---------------------------
void pixelShader(inout Fragment frag)
{	
	frag.color = vec4(fNormal, 1.f);
	frag.normal = fNormal;
	frag.position = fPosition;
	SceneLighting_Lambertian(frag);
}