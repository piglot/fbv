
#include "config.h"
#ifdef YM_SUPPORT_PNG
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "png.h"

#define PNG_BYTES_TO_CHECK 4
#ifndef min
#define min(x,y) ((x) < (y) ? (x) : (y))
#endif

int ym_png_id(char *name)
{
    int fd;
    char id[4];
    fd=open(name,O_RDONLY); if(fd==-1) return(0);
    read(fd,id,4);
    close(fd);
    if(id[1]=='P' && id[2]=='N' && id[3]=='G') return(1);//第一个字节0x89超出了ASCII字符的范围，这是为了避免某些软件将PNG文件当做文本文件来处理                                                           //
    return(0);                                           //第2-4个字节是文件标识符ASCII值为十进制数80,78,71，分别代表“P”“N”“G”三个字符
}
			    

int ym_png_load(char *name,unsigned char *buffer, unsigned char ** alpha,int x,int y)
{
    png_structp png_ptr;
    png_infop info_ptr;
    png_uint_32 width, height;
    int i;
    int bit_depth, color_type, interlace_type;
    int number_passes,pass, trans = 0;
    char *rp;
    png_bytep rptr[2];
    char *fbptr;
    FILE *ym;

    if(!(ym=fopen(name,"rb"))) return(YM_ERROR_FILE);

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if (png_ptr == NULL) return(YM_ERROR_FORMAT);
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
	png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(ym); return(YM_ERROR_FORMAT);
    }
    rp=0;
    if (setjmp(png_ptr->jmpbuf))
    {
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        if(rp) free(rp);
	fclose(ym); return(YM_ERROR_FORMAT);
    }

    png_init_io(png_ptr,ym);

    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,&interlace_type, NULL, NULL);
    if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_expand(png_ptr); 
    if (bit_depth < 8) png_set_packing(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type== PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		trans = 1;
		png_set_tRNS_to_alpha(png_ptr);
	}

    if(bit_depth == 16) png_set_strip_16(png_ptr); 
    number_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr,info_ptr);

	if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA || color_type == PNG_COLOR_TYPE_RGB_ALPHA || trans)
	{
		unsigned char * alpha_buffer = (unsigned char*) malloc(width * height);
		unsigned char * aptr;
		
    	rp = (char*) malloc(width * 4);
	    rptr[0] = (png_bytep) rp;
    
		*alpha = alpha_buffer;
		
	    for (pass = 0; pass < number_passes; pass++)
    	{
			fbptr = buffer;
			aptr = alpha_buffer;
			
			for(i=0; i<height; i++)
    		{
				int n;
				unsigned char *trp = rp;
				
    	   		png_read_rows(png_ptr, rptr, NULL, 1);
				
				for(n = 0; n < width; n++, fbptr += 3, trp += 4)
				{
					memcpy(fbptr, trp, 3);
					*(aptr++) = trp[3];
				}
			}
    	}
	    free(rp);
	}
	else
	{
	    for (pass = 0; pass < number_passes; pass++)
    	{
			fbptr = buffer;
			for(i=0; i<height; i++, fbptr += width*3)
    		{
			    rptr[0] = (png_bytep) fbptr;
    	   		png_read_rows(png_ptr, rptr, NULL, 1);
			}
    	}
	}
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    fclose(ym);
    return(YM_ERROR_OK);
}
int ym_png_getsize(char *name,int *x,int *y)
{
    png_structp png_ptr;
    png_infop info_ptr;
    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type;
    char *rp;
    FILE *ym;

    if(!(ym=fopen(name,"rb"))) return(YM_ERROR_FILE);

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if (png_ptr == NULL) return(YM_ERROR_FORMAT);
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
	png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(ym); return(YM_ERROR_FORMAT);
    }
    rp=0;
    if (setjmp(png_ptr->jmpbuf))
    {
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        if(rp) free(rp);
	fclose(ym); return(YM_ERROR_FORMAT);
    }
   
    png_init_io(png_ptr,ym);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,&interlace_type, NULL, NULL);
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    *x=width;
    *y=height;
    fclose(ym);
    return(YM_ERROR_OK);
}
#endif
