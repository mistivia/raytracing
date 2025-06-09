#ifndef VECMATH_H_
#define VECMATH_H_

#include <stdint.h>

typedef struct {
    int x;
    int y;
} Vec2i;

typedef struct {
    float x;
    float y;
    float z;
} Vec3f;

typedef struct {
    float r;
    float g;
    float b;
} Color;


Vec3f vec3f_sub(Vec3f lhs, Vec3f rhs);
Vec3f vec3f_add(Vec3f lhs, Vec3f rhs);
float vec3f_dot(Vec3f lhs, Vec3f rhs);
Vec3f vec3f_neg(Vec3f v);
Vec3f vec3f_normalize(Vec3f vec);
Vec3f vec3f_mul(float a, Vec3f v);
void vec3f_show(const char *name, Vec3f v);

Color icolor(int32_t rgb);
Color pixel_avg4(Color pixels[4]);

#endif
