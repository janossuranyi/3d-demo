#version 450 core

layout(std140) uniform cb_matrix
{
    mat4 m_W;
    mat4 m_V;
    mat4 m_P;
    mat4 m_Normal;

    mat4 m_WV;
    mat4 m_VP;
    mat4 m_WVP;
    
    mat4 m_iP;
    mat4 m_iVP;    
}

layout(std140) uniform cb_sunlight
{
    vec4 sun_position;
    vec4 sun_direction;
    vec4 sun_color;
    float sun_intensity;
}

layout(std140) uniform cb_camera
{
    vec4 cam_position;
    vec4 cam_target;
    vec4 cam_direction;
    vec4 cam_up;
	float znear;
	float zfar;
	float yfov;
	float ascept;
}

layout(std140) uniform cb_lowFreq
{
    vec4 g_lowFreq[32];
}

layout(std140) uniform cb_highFreq
{
    vec4 g_highFreq[32];
}

// Material Flags
#define MF_METALLIC_ROUGNESS (1)
#define MF_SPECULAR_GLOSSINESS (1<<1)
#define MF_DIFFUSE_TEX (1<<8)
#define MF_NORMAL_TEX (1<<9)
#define MF_METALLIC_ROUGHNESS_TEX (1<<10)
#define MF_SPECULAR_GLOSSINESS_TEX (1<<11)
#define MF_EMISSIVE_TEX (1<<12)
#define Mf_OCCLUSION_TEX (1<<13)

layout(std140) uniform cb_material
{
    vec4 baseColor;
    vec4 specularColor;
    vec4 emissiveColor;
    float param1;   // metalness
    float param2;   // roughness/shininess
    uint flags;         

} material;
