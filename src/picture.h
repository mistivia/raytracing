#ifndef PICTURE_H_
#define PICTURE_H_

#include <stdbool.h>

#include "vecmath.h"


typedef struct {
    int width;
    int height;
    float *buffer;
} Picture;

Picture new_picture(int width, int height);
void delete_picture(Picture pic);

void set_pixel(Picture pic, Vec2i pos, Color c);
void normalize_picture(Picture pic);
Picture picture_downscale_2x(Picture pic);

bool writeBMP(const char* filename, Picture pic);

#endif

