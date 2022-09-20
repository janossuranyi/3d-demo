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
