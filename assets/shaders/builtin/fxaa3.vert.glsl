layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_TexCoord;

const vec2 vertices[3]=vec2[3](vec2(-1,-1), vec2(3,-1), vec2(-1, 3));

out INTERFACE
{
    vec2 uv;
} Out;

void main()
{
    gl_Position= in_Position;
    Out.uv = in_TexCoord;
//    vec4 posCS = vec4(vertices[gl_VertexID],0,1);
//    Out.uv = 0.5 * posCS.xy + vec2(0.5);
//    gl_Position = posCS;
}