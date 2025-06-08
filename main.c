#include <limits.h>
#include <float.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

typedef enum {
    kRetSucc,
    kRetFail,
} Ret;

typedef struct {
    int width;
    int height;
    float *buffer;
} Picture;

typedef struct {
    float r;
    float g;
    float b;
} Color;

typedef struct {
    int x;
    int y;
} Vec2i;


Picture new_picture(int width, int height) {
    Picture ret;
    ret.width = width;
    ret.height = height;
    ret.buffer = malloc(sizeof(float) * width * height * 3);
    return ret;
}

void delete_picture(Picture pic) {
    free(pic.buffer);
}

void set_pixel(Picture pic, Vec2i pos, Color c) {
    int idx = pos.y * pic.width * 3 + pos.x * 3;
    pic.buffer[idx] = c.b;
    pic.buffer[idx+1] = c.g;
    pic.buffer[idx+2] = c.r;
}

Ret fwrite_word16le(FILE* fp, uint16_t x) {
    uint8_t buf[2];
    buf[0] = x & 0xff;
    buf[1] = (x >> 8) & 0xff;
    int r = fwrite(buf, 1, 2, fp);
    if (r != 2) return kRetFail;
    return kRetSucc;
}

Ret fwrite_word32le(FILE* fp, uint32_t x) {
    Ret ret;
    uint16_t buf[2];

    buf[0] = x & 0xffff;
    buf[1] = (x >> 16) & 0xffff;
    ret = fwrite_word16le(fp, buf[0]);
    if (ret != kRetSucc) return ret;
    ret = fwrite_word16le(fp, buf[1]);
    if (ret != kRetSucc) return ret;
    return kRetSucc;
}


Ret writeBMP(const char* filename, Picture pic) {
    Ret ret = kRetFail;
    FILE *fp = NULL;
    uint8_t *databuf = NULL;

    fp = fopen(filename, "wb");
    if (!fp) goto end;

    int row_data_len = 3 * pic.width;
    int padding = (4 - row_data_len % 4) % 4;
    int row_len = row_data_len + padding;
    int img_len = row_len * pic.height;
    int file_len = 53 + img_len;

    ret = fwrite_word16le(fp, 0x4d42);
    if (ret != kRetSucc) goto end;
    ret = fwrite_word32le(fp, file_len);
    if (ret != kRetSucc) goto end;
    ret = fwrite_word32le(fp, 0);
    if (ret != kRetSucc) goto end;
    ret = fwrite_word32le(fp, 54);
    if (ret != kRetSucc) goto end;

    ret = fwrite_word32le(fp, 40);
    if (ret != kRetSucc) goto end;
    ret = fwrite_word32le(fp, pic.width);
    if (ret != kRetSucc) goto end;
    ret = fwrite_word32le(fp, -pic.height);
    if (ret != kRetSucc) goto end;
    ret = fwrite_word16le(fp, 1);
    if (ret != kRetSucc) goto end;
    ret = fwrite_word16le(fp, 24);
    if (ret != kRetSucc) goto end;
    ret = fwrite_word32le(fp, 0); // BI_RGB
    if (ret != kRetSucc) goto end;
    ret = fwrite_word32le(fp, img_len);
    if (ret != kRetSucc) goto end;
    ret = fwrite_word32le(fp, 0);
    if (ret != kRetSucc) goto end;
    ret = fwrite_word32le(fp, 0);
    if (ret != kRetSucc) goto end;
    ret = fwrite_word32le(fp, 0);
    if (ret != kRetSucc) goto end;
    ret = fwrite_word32le(fp, 0);
    if (ret != kRetSucc) goto end;

    databuf = malloc(img_len);
    memset(databuf, 0, img_len);
    for (int i = 0; i < pic.height; i++) {
        for (int j = 0; j < pic.width * 3; j++) {
            int fromidx = i * pic.width * 3 + j;
            int toidx = i * row_len + j;
            float value = pic.buffer[fromidx];
            if (value < 0) databuf[toidx] = 0;
            else if (value > 1.0) {
                databuf[toidx] = 255;
            } else databuf[toidx] = 255 * value;
        }
    }
    if (fwrite(databuf, 1, img_len, fp) != img_len) {
        ret = kRetFail;
        goto end;
    }
    ret = kRetSucc;

end:
    if (databuf != NULL) free(databuf);
    if (fp != NULL) fclose(fp);
    return ret;
}

typedef struct {
    float x;
    float y;
    float z;
} Vec3f;

Vec3f screen_proj(int width, int height, Vec2i sp) {
    Vec3f ret;
    float maxlen = width > height ? width : height; 
    ret.z = 1;
    ret.x = (sp.x - width / 2.0) / maxlen;
    ret.y = (-sp.y + height / 2.0) / maxlen;
    return ret;
}

Color icolor(int r, int g, int b) {
    return (Color){.r = r/255.0, .g = g/255.0, .b = b/255.0};
}

typedef struct {
    Vec3f center;
    float r;
    Color color;
} Ball;

Ball balls[] = {
    {
        .center = {.x = 0, .y = -1, .z = 3},
        .r = 1,
    },
    {
        .center = {.x = 2, .y = 0, .z = 4},
        .r = 1,
    },
    {
        .center = {.x = -2, .y = 0, .z = 4},
        .r = 1,
    },
};


Color gBackgroupColor = {0,0,0};

void init_color() {
    balls[0].color = icolor(0x95, 0xe1, 0xe3);
    balls[1].color = icolor(0xfc, 0xe3, 0x8a);
    balls[2].color = icolor(0xf3, 0x81, 0x81);
    gBackgroupColor = icolor(0xea, 0xea, 0xea);
}

Vec3f vec3f_sub(Vec3f lhs, Vec3f rhs) {
    return (Vec3f){
        .x = lhs.x - rhs.x,
        .y = lhs.y - rhs.y,
        .z = lhs.z - rhs.z,
    };
}

float vec3f_dot(Vec3f lhs, Vec3f rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
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
    float t1 = (-b + sqrt(delta)) / 2*a;
    float t2 = (-b - sqrt(delta)) / 2*a;
    return t1 < t2 ? t1 : t2;
}

Color calc_color(Vec3f v) {
    Vec3f start = {.x = 0, .y = 0, .z = 0};
    float tmin = 0.1;
    float tmax = FLT_MAX;

    int nearest_idx = -1;
    float t_nearest = FLT_MAX;
    for (int i = 0; i < sizeof(balls) / sizeof(Ball); i++) {
        float t = ball_intersect(start, v, &balls[i]);
        // printf("t:%f\n", t);
        if (t < t_nearest && t < tmax && t > tmin) {
            t_nearest = t;
            nearest_idx = i;
        }
    }
    if (nearest_idx >= 0) {
        return balls[nearest_idx].color;
    } else {
        return gBackgroupColor;
    }
}

void normalize_picture(Picture pic) {
    float maxval = 0;
    for (size_t i = 0; i < pic.width * pic.height * 3; i++) {
        float val = pic.buffer[i];
        if (val < 0) val = 0;
        if (val > maxval) maxval = val;
    }
    if (maxval < 1.0) return;
    for (size_t i = 0; i < pic.width * pic.height * 3; i++) {
        pic.buffer[i] = pic.buffer[i] / maxval;
    }
}

int main() {
    init_color();
    int img_w = 1280;
    int img_h = 720;
    Picture pic = new_picture(img_w, img_h);
    for (int x = 0; x < img_w; x++) {
        for (int y = 0; y < img_h; y++) {
            Vec2i screen_pos = {x, y};
            Vec3f v = screen_proj(img_w, img_h, screen_pos);
            set_pixel(pic, screen_pos, calc_color(v));
        }
    }
    writeBMP("test.bmp", pic);
    delete_picture(pic);
    return 0;
}

