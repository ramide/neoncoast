#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static unsigned char pixel[32][32][4];

static void set_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    if (x < 0 || x >= 32 || y < 0 || y >= 32) return;
    pixel[y][x][0] = b;
    pixel[y][x][1] = g;
    pixel[y][x][2] = r;
    pixel[y][x][3] = a;
}

int main(void) {
    for (int y = 0; y < 32; y++)
        for (int x = 0; x < 32; x++)
            set_pixel(x, y, 0, 0, 0, 0);

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            float dx = (float)(x - 16);
            float dy = (float)(y - 16);
            float dist = (float)sqrt(dx * dx + dy * dy);

            if (dist < 6.5f) {
                float bright = 1.0f - dist / 6.5f;
                set_pixel(x, y, (unsigned char)(255 * bright), (unsigned char)(255 * bright),
                          (unsigned char)(255 * bright), (unsigned char)(200 * bright));
            } else if (dist < 8.5f) {
                set_pixel(x, y, 10, 0, 25, 0);
            } else if (dist < 11.5f) {
                set_pixel(x, y, 255, 50, 150, 255);
            } else if (dist < 13.0f) {
                set_pixel(x, y, 10, 0, 25, 0);
            } else if (dist < 15.5f) {
                float fade = 1.0f - (dist - 13.0f) / 2.5f;
                set_pixel(x, y, 0, (unsigned char)(200 * fade), (unsigned char)(255 * fade), (unsigned char)(255 * fade));
            } else if (dist < 17.0f) {
                float glow = 1.0f - (dist - 15.5f) / 1.5f;
                set_pixel(x, y, 0, (unsigned char)(100 * glow), (unsigned char)(200 * glow), (unsigned char)(60 * glow));
            }
        }
    }

    FILE *f = fopen("neoncoast.ico", "wb");
    if (!f) { perror("fopen"); return 1; }

    unsigned char header[] = { 0, 0, 1, 0, 1, 0 };
    fwrite(header, 1, 6, f);

    int pixelOffset = 22;
    int pixelSize = 32 * 32 * 4;
    unsigned char dir[] = {
        32, 32, 0, 0, 1, 0, 32, 0,
        (unsigned char)(pixelSize & 0xFF),
        (unsigned char)((pixelSize >> 8) & 0xFF),
        (unsigned char)((pixelSize >> 16) & 0xFF),
        (unsigned char)((pixelSize >> 24) & 0xFF),
        (unsigned char)(pixelOffset & 0xFF),
        (unsigned char)((pixelOffset >> 8) & 0xFF),
        (unsigned char)((pixelOffset >> 16) & 0xFF),
        (unsigned char)((pixelOffset >> 24) & 0xFF)
    };
    fwrite(dir, 1, 16, f);

    for (int y = 31; y >= 0; y--)
        for (int x = 0; x < 32; x++)
            fwrite(&pixel[y][x][0], 1, 4, f);

    fclose(f);
    printf("Generated neoncoast.ico (%d bytes)\n", pixelOffset + pixelSize);
    return 0;
}
