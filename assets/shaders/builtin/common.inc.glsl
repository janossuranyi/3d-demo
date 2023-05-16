#define	IMU_DIFFUSE 0
#define	IMU_NORMAL 1
#define IMU_AORM 2
#define IMU_EMMISIVE 3
#define IMU_DEPTH 4
#define IMU_FRAGPOS 5
#define IMU_HDR 6
#define IMU_DEFAULT 7

float saturate(float x) { return clamp(x, 0.0, 1.0); }
vec2 saturate(vec2 x) { return clamp(x, 0.0, 1.0); }
vec3 saturate(vec3 x) { return clamp(x, 0.0, 1.0); }
vec4 saturate(vec4 x) { return clamp(x, 0.0, 1.0); }
