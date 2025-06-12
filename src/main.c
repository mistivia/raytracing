#include <limits.h>
#include <float.h>
#include <stdint.h>
#include <math.h>

#include "vecmath.h"
#include "picture.h"

#define EPSILON 0.001

typedef enum {
    kAmbientLight,
    kPointLight,
    kDirectionalLight,
} LightType;

typedef struct {
    LightType type;
    float intensity;
    Vec3f vec;
} Light;

Vec3f screen_proj(int width, int height, Vec2i sp) {
    Vec3f ret;
    float maxlen = width > height ? width : height; 
    ret.z = 1.0;
    ret.x = (sp.x - width / 2.0) / maxlen;
    ret.y = (-sp.y + height / 2.0) / maxlen;
    return ret;
}

typedef struct {
    Vec3f center;
    float r;
    Color color;
    float specular;
} Ball;

Ball balls[] = {
    {
        .center = {.x = 0, .y = -1, .z = 3},
        .r = 1,
        .specular = 500,
    },
    {
        .center = {.x = 2, .y = 0, .z = 4},
        .r = 1,
        .specular = 500,
    },
    {
        .center = {.x = -2, .y = 0, .z = 4},
        .r = 1,
        .specular = 10,
    },
    {
        .center = {0, -5001, 0},
        .r = 5000,
        .specular = 1000,
    },
};

Light lights[] = {
    {
        .type = kAmbientLight,
        .intensity = 0.2,
    },
    {
        .type = kPointLight,
        .intensity = 0.6,
        .vec = {2.0, 1.0, 0.0},
    },
    {
        .type = kDirectionalLight,
        .intensity = 0.2,
        .vec = {1.0, 4.0, 4.0}
    },
};


Color gBackgroupColor = {0,0,0};

void init_color() {
    balls[0].color = icolor(0x95e1d3);
    balls[1].color = icolor(0xfce38a);
    balls[2].color = icolor(0xf38181);
    balls[3].color = icolor(0xeaffd0);
    gBackgroupColor = icolor(0xeaeaea);
}

float ball_intersect(Vec3f start, Vec3f ray, Ball *ball) {
    Vec3f sc = vec3f_sub(start, ball->center);
    float a = vec3f_dot(ray, ray);
    float b = 2 * vec3f_dot(sc, ray);
    float c = vec3f_dot(sc, sc) - ball->r * ball->r;
    float delta = b*b - 4*a*c;
    if (delta < 0) {
        return -1;
    }
    float t1 = (-b + sqrt(delta)) / (2*a);
    float t2 = (-b - sqrt(delta)) / (2*a);
    return t1 < t2 ? t1 : t2;
}

bool is_in_shadow(Vec3f pos, Light light) {
    Vec3f ray;
    if (light.type == kPointLight) {
        ray = vec3f_sub(light.vec, pos);
    } else if (light.type == kDirectionalLight) {
        ray = light.vec;
    } else {
        return false;
    }
    float tmin = FLT_MAX;
    for (int i = 0; i < sizeof(balls) / sizeof(Ball); i++) {
        float t = ball_intersect(pos, ray, &balls[i]);
        if (t > EPSILON && t < tmin) tmin = t;
    }
    if (light.type == kPointLight) {
        return tmin < 1;
    }
    if (light.type == kDirectionalLight) {
        return tmin != FLT_MAX;
    }
    return false;
}

float specular_coeff(Vec3f l, Vec3f n, Vec3f v, float s) {
    Vec3f r = vec3f_sub(vec3f_mul(2 * vec3f_dot(n, l), n), l);
    v = vec3f_normalize(v);
    float prod = vec3f_dot(r, vec3f_neg(v));
    if (prod < 0) return 0;
    else return pow(vec3f_dot(r, vec3f_neg(v)), s);
}


Color ball_surface_color(Ball *ball, Vec3f point, Vec3f view) {
    Vec3f norm = vec3f_normalize(vec3f_sub(point, ball->center));
    float amp = 0;
    for (int i = 0; i < sizeof(lights) / sizeof(Light); i++) {
        if (lights[i].type == kAmbientLight) {
            amp += lights[i].intensity;
        } else if (lights[i].type == kPointLight) {
            if (is_in_shadow(point, lights[i])) continue;
            Vec3f l = vec3f_normalize(vec3f_sub(lights[i].vec, point));
            float prod = vec3f_dot(l, norm);
            if (prod > 0) amp += prod * lights[i].intensity;
            if (ball->specular > 0) {
                amp += specular_coeff(l, norm, view, ball->specular)
                        * lights[i].intensity;
            }
        } else if (lights[i].type == kDirectionalLight) {
            if (is_in_shadow(point, lights[i])) continue;
            Vec3f l = vec3f_normalize(lights[i].vec);
            float prod = vec3f_dot(l, norm);
            if (prod > 0) amp += prod * lights[i].intensity;
            if (ball->specular > 0) {
                amp += specular_coeff(l, norm, view, ball->specular)
                        * lights[i].intensity;
            }
        }
    }
    return (Color){
        ball->color.r * amp,
        ball->color.g * amp,
        ball->color.b * amp
    };
}

Color calc_color(Vec3f v) {
    Vec3f start = {.x = 0, .y = 0, .z = 0};
    float tmin = 0.1;
    float tmax = FLT_MAX;

    int nearest_idx = -1;
    float t_nearest = FLT_MAX;
    for (int i = 0; i < sizeof(balls) / sizeof(Ball); i++) {
        float t = ball_intersect(start, v, &balls[i]);
        if (t < t_nearest && t < tmax && t > tmin) {
            t_nearest = t;
            nearest_idx = i;
        }
    }
    if (nearest_idx >= 0) {
        Vec3f intersection = vec3f_add(start, vec3f_mul(t_nearest, v));
        return ball_surface_color(&balls[nearest_idx], intersection, v);
    } else {
        return gBackgroupColor;
    }
}


int main() {
    init_color();
    int img_w = 800*2;
    int img_h = 800*2;
    Picture pic = new_picture(img_w, img_h);
    for (int x = 0; x < img_w; x++) {
        for (int y = 0; y < img_h; y++) {
            Vec2i screen_pos = {x, y};
            Vec3f v = screen_proj(img_w, img_h, screen_pos);
            set_pixel(pic, screen_pos, calc_color(v));
        }
    }
    Picture newpic = picture_downscale_2x(pic);
    writeBMP("test.bmp", newpic);
    delete_picture(pic);
    delete_picture(newpic);
    return 0;
}

