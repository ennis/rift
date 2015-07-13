#pragma lighting_shader Phong IN_LIGHT OUT_LIGHT
#pragma input vec3 diffuse (required)
#pragma input vec3 wsNormal (required)
#pragma input vec3 wsTangent (optional: HAS_TANGENT)
#pragma input vec3 wsBitangent (optional: HAS_BITANGENT)

#pragma constant sampler2D shadowMap (optional: HAS_SHADOWMAP)
#pragma constant sampler2D environmentMap (optional: HAS_ENVMAP)
#pragma constant vec3 ambient (required, default = vec3(0.5))

// light struct
#pragma include <light.glsl>