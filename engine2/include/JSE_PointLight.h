#ifndef JSE_POINT_LIGHT_H
#define JSE_POINT_LIGHT_H

struct PointLight {
	vec4 color;
	vec4 position;
	vec4 paddingAndRadius;
};

#endif