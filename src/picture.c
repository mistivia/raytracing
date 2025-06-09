#include "picture.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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


static bool fwrite_word16le(FILE* fp, uint16_t x) {
    uint8_t buf[2];
    buf[0] = x & 0xff;
    buf[1] = (x >> 8) & 0xff;
    int r = fwrite(buf, 1, 2, fp);
    if (r != 2) return false;
    return true;
}

static bool fwrite_word32le(FILE* fp, uint32_t x) {
    bool ret = false;
    uint16_t buf[2];

    buf[0] = x & 0xffff;
    buf[1] = (x >> 16) & 0xffff;
    ret = fwrite_word16le(fp, buf[0]);
    if (!ret) return ret;
    ret = fwrite_word16le(fp, buf[1]);
    if (!ret) return ret;
    return true;
}


bool writeBMP(const char* filename, Picture pic) {
    bool ret = false;
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
    if (!ret) goto end;
    ret = fwrite_word32le(fp, file_len);
    if (!ret) goto end;
    ret = fwrite_word32le(fp, 0);
    if (!ret) goto end;
    ret = fwrite_word32le(fp, 54);
    if (!ret) goto end;

    ret = fwrite_word32le(fp, 40);
    if (!ret) goto end;
    ret = fwrite_word32le(fp, pic.width);
    if (!ret) goto end;
    ret = fwrite_word32le(fp, -pic.height);
    if (!ret) goto end;
    ret = fwrite_word16le(fp, 1);
    if (!ret) goto end;
    ret = fwrite_word16le(fp, 24);
    if (!ret) goto end;
    ret = fwrite_word32le(fp, 0); // BI_RGB
    if (!ret) goto end;
    ret = fwrite_word32le(fp, img_len);
    if (!ret) goto end;
    ret = fwrite_word32le(fp, 0);
    if (!ret) goto end;
    ret = fwrite_word32le(fp, 0);
    if (!ret) goto end;
    ret = fwrite_word32le(fp, 0);
    if (!ret) goto end;
    ret = fwrite_word32le(fp, 0);
    if (!ret) goto end;
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
        ret = false;
        goto end;
    }
    ret = true;

end:
    if (databuf != NULL) free(databuf);
    if (fp != NULL) fclose(fp);
    return ret;
}
