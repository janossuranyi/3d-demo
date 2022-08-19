#version 410 core

layout(std140) uniform Matrixes {
    mat4 rpMtxInvProj;
    mat4 rpMtxModelViewProj;
    mat4 rpMtxModel;
    mat4 rpMtxNormal;
};

struct SpotLight {
    vec4 color;
    vec4 attenuation;
    vec4 position;
    vec4 spotDir;
    float innerConeAngle;
    float outerConeAngle;
};

struct DirLight {
    vec4 color;
    vec4 direction;
};

struct PointLight {
    vec4 color;
    vec4 attenuation;
    vec4 position;
};

layout(std140) uniform SpotLights {
    SpotLight rpSpotLights[256];
};

layout(std140) uniform PointLights {
    PointLight rpPointLights[256];
};

layout(std140) uniform DirLights {
    DirLight rpDirLights[256];
};

layout(std140) uniform Params {
    vec4    rpEyePos;
    int     rpNumSpotLight;
    int     rpNumDirLight;
    int     rpNumPointLight;
};
