#include "vecmath.h"

#include <math.h>
#include <stdio.h>

Vec3f vec3f_sub(Vec3f lhs, Vec3f rhs) {
    return (Vec3f){
        .x = lhs.x - rhs.x,
        .y = lhs.y - rhs.y,
        .z = lhs.z - rhs.z,
    };
}

Vec3f vec3f_add(Vec3f lhs, Vec3f rhs) {
    return (Vec3f){
        .x = lhs.x + rhs.x,
        .y = lhs.y + rhs.y,
        .z = lhs.z + rhs.z,
    };
}

float vec3f_dot(Vec3f lhs, Vec3f rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

Vec3f vec3f_neg(Vec3f v) {
    return (Vec3f){-v.x, -v.y, -v.z};
}

Vec3f vec3f_normalize(Vec3f vec) {
    float len = sqrt(vec3f_dot(vec, vec));
    return (Vec3f){vec.x/len, vec.y/len, vec.z/len};
}

Vec3f vec3f_mul(float a, Vec3f v) {
    return (Vec3f){v.x * a, v.y * a, v.z * a};
}

Color icolor(int32_t rgb) {
    int r = (rgb >> 16) & 0xff;
    int g = (rgb >> 8) & 0xff;
    int b = rgb & 0xff;
    return (Color){.r = r/255.0, .g = g/255.0, .b = b/255.0};
}

void vec3f_show(const char *name, Vec3f v) {
    printf("%s(%f,%f,%f)\n", name, v.x, v.y, v.z);
}

Color pixel_avg4(Color pixels[4]) {
    float r = 0,g = 0,b = 0;
    for (int i = 0; i < 4; i++) {
        r += pixels[i].r; 
        g += pixels[i].g; 
        b += pixels[i].b; 
    }
    return (Color) {r/4.0, g/4.0, b/4.0};
}
