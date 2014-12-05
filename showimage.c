#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <asm/types.h>
#include <string.h>
#include <errno.h>
#include "config.h"
#include "type.h"
/* Public Use Functions:
 *
 * extern void showimage(unsigned char *rgbbuff,
 *     int x_size, int y_size,
 *     int x_pan, int y_pan,
 *     int x_offs, int y_offs);
 *
 * extern void getCurrentRes(int *x,int *y);
 *
 */

unsigned short red[256], green[256], blue[256];
struct fb_cmap map332 = {0, 256, red, green, blue, NULL};
unsigned short red_b[256], green_b[256], blue_b[256];
struct fb_cmap map_back = {0, 256, red_b, green_b, blue_b, NULL};


int openFB(const char *name);
void closeFB(int ym);
void getVarScreenInfo(int ym, struct fb_var_screeninfo *var);
void setVarScreenInfo(int ym, struct fb_var_screeninfo *var);
void getFixScreenInfo(int ym, struct fb_fix_screeninfo *fix);
void set332map(int ym);
void* convertRGB2FB(int ym, unsigned char *rgbbuff, unsigned long count, int bpp, int *cpp);
void blit2FB(int ym, void *fbbuff, unsigned char *alpha,
	unsigned int pic_xs, unsigned int pic_ys,
	unsigned int scr_xs, unsigned int scr_ys,
	unsigned int xp, unsigned int yp,
	unsigned int xoffs, unsigned int yoffs,
	int cpp);

void showimage(unsigned char *rgbbuff, unsigned char * alpha, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs)
{
    struct fb_var_screeninfo var;
    struct fb_fix_screeninfo fix;
    unsigned short *fbbuff = NULL;
    int ym = -1, bp = 0;
    unsigned long x_stride;
    
    /* get the framebuffer device handle */
    ym = openFB(NULL);
    
    /* read current video mode */
    getVarScreenInfo(ym, &var);
    getFixScreenInfo(ym, &fix);
    
    x_stride = (fix.line_length * 8) / var.bits_per_pixel;
   
    /* correct panning */
    if(x_pan > x_size - x_stride) x_pan = 0;
    if(y_pan > y_size - var.yres) y_pan = 0;
    /* correct offset */
    if(x_offs + x_size > x_stride) x_offs = 0;
    if(y_offs + y_size > var.yres) y_offs = 0;
    
    /* blit buffer 2 fb */
    fbbuff = convertRGB2FB(ym, rgbbuff, x_size * y_size, var.bits_per_pixel, &bp);
#if 0
    blit2FB(ym, fbbuff, alpha, x_size, y_size, x_stride, var.yres, x_pan, y_pan, x_offs, y_offs, bp);
#else
    blit2FB(ym, fbbuff, alpha, x_size, y_size, x_stride, var.yres_virtual, x_pan, y_pan, x_offs, y_offs + var.yoffset, bp);
#endif
    free(fbbuff);
   
    /* close device */
    closeFB(ym);
}

void getCurrentRes(int *x, int *y)
{
    struct fb_var_screeninfo var;
    int ym = -1;
    ym = openFB(NULL);
    getVarScreenInfo(ym, &var);
    *x = var.xres;
    *y = var.yres;
    closeFB(ym);
}

int openFB(const char *name)
{
    int ym;
    char *dev;

    if(name == NULL)
    {
      dev = getenv("FRAMEBUFFER");//getenv是声明在stdlib.h中的函数,getenv()用来取得参数环境变量的内容。
      if(dev)
        name = dev;
      else name = DEFAULT_FRAMEBUFFER;
    }
    
    if ((ym = open(name, O_RDWR)) == -1)
    {
        fprintf(stderr, "open %s: %s\n", name, strerror(errno));
        exit(1);
    }
    
    return ym;
}

void closeFB(int ym)
{   
    close(ym);
}

void getVarScreenInfo(int ym, struct fb_var_screeninfo *var)
{
    if (ioctl(ym, FBIOGET_VSCREENINFO, var))
    {
        fprintf(stderr, "ioctl FBIOGET_VSCREENINFO: %s\n", strerror(errno));
        exit(1);
    }
}

void setVarScreenInfo(int ym, struct fb_var_screeninfo *var)
{
    if (ioctl(ym, FBIOPUT_VSCREENINFO, var))
    {
        fprintf(stderr, "ioctl FBIOPUT_VSCREENINFO: %s\n", strerror(errno));
        exit(1);
    }
}

void getFixScreenInfo(int ym, struct fb_fix_screeninfo *fix)
{
    if (ioctl(ym, FBIOGET_FSCREENINFO, fix))
    {
        fprintf(stderr, "ioctl FBIOGET_FSCREENINFO: %s\n", strerror(errno));
        exit(1);
    }
}

void make332map(struct fb_cmap *map)
{
	int rs, gs, bs, i;
	int r = 8, g = 8, b = 4;

	map->red = red;
	map->green = green;
	map->blue = blue;

	rs = 256 / (r - 1);
	gs = 256 / (g - 1);
	bs = 256 / (b - 1);
	
	for (i = 0; i < 256; i++) {
		map->red[i]   = (rs * ((i / (g * b)) % r)) * 255;
		map->green[i] = (gs * ((i / b) % g)) * 255;
		map->blue[i]  = (bs * ((i) % b)) * 255;
	}
}

void set8map(int ym, struct fb_cmap *map)
{
    if (ioctl(ym, FBIOPUTCMAP, map) < 0) {
        YM_ERR( "Error putting colormap !");
        exit(1);
    }
}

void get8map(int ym, struct fb_cmap *map)
{
    if (ioctl(ym, FBIOGETCMAP, map) < 0) {
        YM_ERR("Error getting colormap !");
        exit(1);
    }
}

void set332map(int ym)
{
    make332map(&map332);
    set8map(ym, &map332);
}

void blit2FB(int ym, void *fbbuff, unsigned char *alpha,
	unsigned int pic_xs, unsigned int pic_ys,
	unsigned int scr_xs, unsigned int scr_ys,
	unsigned int xp, unsigned int yp,
	unsigned int xoffs, unsigned int yoffs,
	int cpp)
{
  int i, xc, yc;
	unsigned char *fb;
	
	unsigned char *fbptr;
	unsigned char *imptr;
	
    xc = (pic_xs > scr_xs) ? scr_xs : pic_xs;
    yc = (pic_ys > scr_ys) ? scr_ys : pic_ys;
    
	fb = mmap(NULL, scr_xs * scr_ys * cpp, PROT_WRITE | PROT_READ, MAP_SHARED, ym, 0);
	/*该函数声明在#include <sys/mman.h>中
	void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offsize);
      建立内存映射:将某个文件内容映射到内存中，对该内存区域的存取即是对该文件内容的读写。
      参数start指向欲对应的内存起始地址，通常设为NULL,代表让系统自动选定地址，对应成功后该地址会返回。
      参数length代表将文件中多大的部分对应到内存。
      参数prot代表映射区域的保护方式。有下列组合:PROT_EXEC 映射区域可被执行，PROT_READ 映射区域可读 ，PROT_WRITE 映射区域可写，PROT_NONE 映射区域不能存取。
			参数flags会影响映射区域的各种特性。其中，MAP_SHARED:对映射区域的写入数据会复制回文件中，而且允许其他映射该文件的进程共享。
			参数fd为open（）返回的文件描述符，代表欲映射到内存的文件。
			参数offset为文件映射的偏移量，通常设置为0，代表从文件最前方开始对应。
	*/
	if(fb == MAP_FAILED)
	{
		perror("mmap");
		return;
	}

	if(cpp == 1)
	{
	    get8map(ym, &map_back);
	    set332map(ym);
	}

	fbptr = fb     + (yoffs * scr_xs + xoffs) * cpp;
	imptr = fbbuff + (yp	* pic_xs + xp) * cpp;
	
	if(alpha)
	{
	 	unsigned char * alphaptr;
		int from, to, x;
		
		alphaptr = alpha + (yp	* pic_xs + xp);
		
		for(i = 0; i < yc; i++, fbptr += scr_xs * cpp, imptr += pic_xs * cpp, alphaptr += pic_xs)
		{
			for(x = 0; x<xc; x++)
			{
				int v;
				
				from = to = -1;
				for(v = x; v<xc; v++)
				{
					if(from == -1)
					{
						if(alphaptr[v] > 0x80) from = v;
					}
					else
					{
						if(alphaptr[v] < 0x80)
						{
							to = v;
							break;
						}
					}
				}
				if(from == -1)
					break;
					
				if(to == -1) to = xc;
				
				memcpy(fbptr + (from * cpp), imptr + (from * cpp), (to - from - 1) * cpp);
				x += to - from - 1;
			}
		}
	}
	else
	    for(i = 0; i < yc; i++, fbptr += scr_xs * cpp, imptr += pic_xs * cpp)
			memcpy(fbptr, imptr, xc * cpp);
		
	if(cpp == 1)
	    set8map(ym, &map_back);
	
	munmap(fb, scr_xs * scr_ys * cpp);//解除内存映射
}

inline static unsigned char make8color(unsigned char r, unsigned char g, unsigned char b)
{
    return (
	(((r >> 5) & 7) << 5) |
	(((g >> 5) & 7) << 2) |
	 ((b >> 6) & 3)       );
}

inline static unsigned short make15color(unsigned char r, unsigned char g, unsigned char b)
{
    return (
	(((r >> 3) & 31) << 10) |
	(((g >> 3) & 31) << 5)  |
	 ((b >> 3) & 31)        );
}

inline static unsigned short make16color(unsigned char r, unsigned char g, unsigned char b)
{
    return (
	(((r >> 3) & 31) << 11) |
	(((g >> 2) & 63) << 5)  |
	 ((b >> 3) & 31)        );
}

void* convertRGB2FB(int ym, unsigned char *rgbbuff, unsigned long count, int bpp, int *cpp)
{ 
    unsigned long i;
    void *fbbuff = NULL;
	  u_int8_t  *c_fbbuff;
    u_int16_t *s_fbbuff;
    u_int32_t *i_fbbuff;

    switch(bpp)
    {
	case 8:
	    *cpp = 1;
	    c_fbbuff = (unsigned char *) malloc(count * sizeof(unsigned char));
	    for(i = 0; i < count; i++)
		c_fbbuff[i] = make8color(rgbbuff[i*3], rgbbuff[i*3+1], rgbbuff[i*3+2]);
	    fbbuff = (void *) c_fbbuff;
	    break;
	case 15:
	    *cpp = 2;
	    s_fbbuff = (unsigned short *) malloc(count * sizeof(unsigned short));
	    for(i = 0; i < count ; i++)
		s_fbbuff[i] = make15color(rgbbuff[i*3], rgbbuff[i*3+1], rgbbuff[i*3+2]);
	    fbbuff = (void *) s_fbbuff;
	    break;
	case 16:
	    *cpp = 2;
	    s_fbbuff = (unsigned short *) malloc(count * sizeof(unsigned short));
	    for(i = 0; i < count ; i++)
		s_fbbuff[i] = make16color(rgbbuff[i*3], rgbbuff[i*3+1], rgbbuff[i*3+2]);
	    fbbuff = (void *) s_fbbuff;
	    break;
	case 24:
	case 32:
	    *cpp = 4;
	    i_fbbuff = (unsigned int *) malloc(count * sizeof(unsigned int));
	    for(i = 0; i < count ; i++)
		i_fbbuff[i] = ((rgbbuff[i*3] << 16) & 0xFF0000) |
			    ((rgbbuff[i*3+1] << 8) & 0xFF00) |
			    (rgbbuff[i*3+2] & 0xFF);
	    fbbuff = (void *) i_fbbuff;
	    break;
	default:
	    YM_ERR("Unsupported video mode! You've got: %dbpp\n", bpp);
	    exit(1);
    }
    return fbbuff;
}
