#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef enum {
    kRetSucc,
    kRetFail,
} Ret;

typedef struct {
    int width;
    int height;
    float *buffer;
} Picture;


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

void set_pixel(Picture pic, int x, int y, float r, float g, float b) {
    int idx = y * pic.width * 3 + x * 3;
    pic.buffer[idx] = b;
    pic.buffer[idx+1] = g;
    pic.buffer[idx+2] = r;
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

int main() {
    Picture pic = new_picture(1600, 900);
    for (int i = 0; i < 900; i++) {
        for (int j = 0; j < 1600; j++) {
            float r = (i + j) % 177 / 177.0;
            float g = (i + j) % 71 / 71.0;
            float b = (i + j) % 337 / 337.0;
            set_pixel(pic, j, i, r, g, b);
        }
    }
    writeBMP("test.bmp", pic);
    delete_picture(pic);
    return 0;
}

