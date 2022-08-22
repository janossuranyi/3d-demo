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
    vec4 cam_near_far_fov;
}

layout(std140) uniform cb_lowFreq
{
    vec4 g_lowFreq[32];
}

layout(std140) uniform cb_highFreq
{
    vec4 g_highFreq[32];
}
