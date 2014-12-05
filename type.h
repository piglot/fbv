
#define YM_ERROR_OK 0
#define YM_ERROR_FILE 1		/* read/access error */
#define YM_ERROR_FORMAT 2	/* file format error */
#define YM_INFO(...)(printf("\033[1;40;32m"),printf(__VA_ARGS__),printf("\033[0m"))
#define YM_ERR(...)(printf("\033[1;40;31m"),printf(__VA_ARGS__),printf("\033[0m"))

int judge_image(char *filename);

void showimage(unsigned char *rgbbuff, unsigned char * alpha, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs);
void getCurrentRes(int *x, int *y);

int ym_bmp_id(char *name);
int ym_bmp_load(char *name,unsigned char *buffer, unsigned char **alpha, int x,int y);
int ym_bmp_getsize(char *name,int *x,int *y);

int ym_jpeg_id(char *name);
int ym_jpeg_load(char *name,unsigned char *buffer, unsigned char **alpha, int x,int y);
int ym_jpeg_getsize(char *name,int *x,int *y);

int ym_png_id(char *name);
int ym_png_load(char *name,unsigned char *buffer, unsigned char **alpha, int x,int y);
int ym_png_getsize(char *name,int *x,int *y);

int ym_gif_id(char *name);
int ym_gif_load(char *name,unsigned char *buffer, unsigned char **alpha, int x,int y);
int ym_gif_getsize(char *name,int *x,int *y);

struct image
{
	int width, height;
	unsigned char *rgb;
	unsigned char *alpha;
	int do_free;
};

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
unsigned char * simple_resize(unsigned char * orgin,int ox,int oy,int dx,int dy);
unsigned char * alpha_resize(unsigned char * alpha,int ox,int oy,int dx,int dy);
unsigned char * color_average_resize(unsigned char * orgin,int ox,int oy,int dx,int dy);
unsigned char * rotate(unsigned char *i, int ox, int oy, int rot);
unsigned char * alpha_rotate(unsigned char *i, int ox, int oy, int rot);
