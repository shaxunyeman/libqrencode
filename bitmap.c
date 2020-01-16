#include <string.h>
#include <stdlib.h>

#include "bitmap.h"

struct Point MakePoint(int x, int y) {
    struct Point p;
    p.x = x;
    p.y = y;
    return p;
}

struct BitMapRect MakeBitMapRect(int w, int h) {
    struct BitMapRect rect;
    rect.width = w;
    rect.heigth = h;
}

struct BitMapContent MakeBitMapContent(int width, int heigth, int bytes_per_pixel) {
    struct BitMapContent ctx;
	int size = 0;
	ctx.width = width;
	ctx.heigth = heigth;
	ctx.bytes_per_pixel = bytes_per_pixel;
	size = BitMapContentSize(&ctx);
	ctx.data = malloc(size);
    // full white color for all pixels
	memset(ctx.data, 0xff, size);
	return ctx;
}

void ReleaseBitMapContent(const struct BitMapContent* ctx) {
    free(ctx->data);
}

int BitMapContentSize(const struct BitMapContent* ctx) {
    int padding = (4 - ((ctx->width * 3) % 4)) % 4;
    return ctx->heigth * (ctx->width *  ctx->bytes_per_pixel + padding);
}

void AddRectToBitMapContent(struct BitMapContent* ctx, const BitMapRect& rect) {
    int width_pixel = rect->width;
    int heigth_pixel = rect->heigth;
    struct point p = rect->origin;
	char* data = ctx->data;
    unsigned char *startRect = &data[p.y * ctx->width * ctx->bytes_per_pixel + p.x * ctx->bytes_per_pixel];

    for (int h = 0; h < heigth_pixel; h++)
    {
        for (int w = 0; w < width_pixel; w++)
        {
			for (int i = 0; i < ctx->bytes_per_pixel; i++) {
				startRect[w * ctx->bytes_per_pixel + i] = 0x00; // full black color for specified pixels
			}
		}
        startRect = &data[(p.y + h + 1) * ctx->width * ctx->bytes_per_pixel + p.x * ctx->bytes_per_pixel];
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma pack(push, 2)
typedef struct  
{
    unsigned short bfType;
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
} BITMAPFILEHEADER;

typedef struct 
{
    unsigned int      biSize;
    signed int       biWidth;
    signed int       biHeight;
    unsigned short       biPlanes;
    unsigned short       biBitCount;
    unsigned int      biCompression;
    unsigned int      biSizeImage;
    signed int       biXPelsPerMeter;
    signed int       biYPelsPerMeter;
    unsigned int      biClrUsed;
    unsigned int      biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)

static FILE *OpenFile(const char *outfile)
{
	FILE *fp;

	if(outfile == NULL || (outfile[0] == '-' && outfile[1] == '\0')) {
		fp = stdout;
	} else {
		fp = fopen(outfile, "wb");
		if(fp == NULL) {
			fprintf(stderr, "Failed to create file: %s\n", outfile);
			perror(NULL);
			exit(EXIT_FAILURE);
		}
	}

	return fp;
}

static int writeFile(FILE* fp, const char* data, int size) {
	int ret;
	ret = fwrite(data, 1, size, fp);
	if(ret == 0) {
		fprintf(stderr, "write data failure.\n");
		exit(EXIT_FAILURE);
	}
	return ret;
}

int SaveBitMap(const char* fileName, struct BitMapContent* ctx) {
    const int wBitsPerPixel = ctx->bytes_per_pixel()*8;
    const int width = ctx->width();
    const int heigth = ctx->heigth();
    const unsigned char* data = ctx->data();
    const int size = ctx->size();

    BITMAPFILEHEADER kFileHeader;
    kFileHeader.bfType = 0x4d42; // "BM"
    kFileHeader.bfSize = sizeof(BITMAPFILEHEADER) +
                         sizeof(BITMAPINFOHEADER) +
                         size;

    kFileHeader.bfReserved1 = 0;
    kFileHeader.bfReserved2 = 0;
    kFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) +
                            sizeof(BITMAPINFOHEADER);

    BITMAPINFOHEADER kInfoHeader;
    kInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    kInfoHeader.biWidth = width;
    kInfoHeader.biHeight = -heigth;
    kInfoHeader.biPlanes = 1;
    kInfoHeader.biBitCount = wBitsPerPixel;
    kInfoHeader.biCompression = 0;
    kInfoHeader.biSizeImage = 0;
    kInfoHeader.biXPelsPerMeter = 0;
    kInfoHeader.biYPelsPerMeter = 0;
    kInfoHeader.biClrUsed = 0;
    kInfoHeader.biClrImportant = 0;
    
    FILE *pf = OpenFile(fileName);
    writeFile(pf, (const char*)&kFileHeader, sizeof(kFileHeader));
    writeFile(pf, (const char*)&kInfoHeader, sizeof(kInfoHeader));
    writeFile(pf, (const char*)data, size);
    return 0;
}

/////////////////////////////////////////////////////////////////////
// for test
/////////////////////////////////////////////////////////////////////
static void SaveQR(const QRcode * qrcode,int width, int heigth, int bytes_per_pixel, const char *outfile) {
    int margin = 0;
	int pixel_size = 1;
	int qr_width = qrcode->width;
	if (width % qr_width) {
		pixel_size = width / qr_width;
		margin = (width - qr_width * pixel_size) / 2;
	}
    
    struct BitMapContent ctx = MakeBitMapContent(width, heigth, size);
	struct BitMapRect drawRect = MakeBitMapRect(pixel_size, pixel_size);
	unsigned char* qr_data = qrcode->data;
	for (int y = 0; y < qr_width; y++) {
		for (int x = 0; x < qr_width; x++) {
			if (*qr_data & 1) {
				drawRect.origin = MakePoint(margin + x * pixel_size, margin + y * pixel_size);
				AddRectToBitMapContent(&ctx, &drawRect);
			}
			qr_data++;
		}
	}

	SaveBitMap(outfile, &ctx);
    
	ReleaseBitMapContent(&ctx);
}

void TEST_save_QRcode(int width, int heigth, int bytes_per_pixel) {
    QRcode *code;
	code = QRcode_encodeString("hi,world", 0, QR_ECLEVEL_M, QR_MODE_8, 1);
    if (code)
        SaveQR(code, width, heigth, bytes_per_pixel, "/tmp/test.bmp")
}

TEST_save_QRcode(128,128,3);
TEST_save_QRcode(128,128,2);