#version 430 core

out vec4 color;

in VS_OUT {
   vec4 Position;
   vec3 Normal;
} fs_in;

layout(std140, binding = 2) uniform cb_camera
{
    vec4 cam_position;
    vec4 cam_target;
    vec4 cam_direction;
    vec4 cam_up;
	float znear;
	float zfar;
	float yfov;
	float ascept;
};


void main()
{
	vec3 lightPos	= vec3(cam_position);
	vec3 lightColor = vec3(1, 1, 1);
	vec3 diffColor	= vec3(0.4,0.4,0.4);

	vec3 N = normalize(fs_in.Normal);
	vec3 L = normalize( lightPos - vec3(fs_in.Position) );
	vec3 V = vec3( normalize( cam_position - fs_in.Position) );
	vec3 R = reflect(-L, N);

	float cosAlpha = max(0.0, dot(N, L));
	float spec = pow(max(dot(V,R),0.0),32);

	vec3 specular = 0.8 * spec * lightColor;
	vec3 ambient = vec3(0.05, 0.05, 0.2);
	vec3 diffuse = lightColor * cosAlpha;

	color = vec4((ambient + diffuse + specular) * diffColor, 1);

}
