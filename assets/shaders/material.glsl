layout(std140, binding = 5) uniform cb_material
{
    vec4 baseColorFactor;
    vec4 metallicFactor;
    vec4 emissiveFactor;
    float roughnessFactor;
    float normalScale;
    float occlusionStrength;
    float alphaCutoff;
    uint alphamode;
} material;
