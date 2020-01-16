#ifndef BITMAP_H
#define BITMAP_H

struct Point{
    int x;
    int y;
};

struct Point MakePoint(int x, int y);

struct BitMapRect {
    int width;
    int heigth;
    struct Point origin;
};

struct BitMapRect MakeBitMapRect(int w, int h);

struct BitMapContent {
    int width;
    int heigth;
    int bytes_per_pixel;
    char* data;
};

struct BitMapContent MakeBitMapContent(int width, int heigth, int bytes_per_pixel);
void ReleaseBitMapContent(const struct BitMapContent* ctx);
int BitMapContentSize(const struct BitMapContent* ctx);
void AddRectToBitMapContent(struct BitMapContent* ctx, const BitMapRect& rect);

int SaveBitMap(const char* fileName, struct BitMapContent* ctx);

#endif // BITMAP_H