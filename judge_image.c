#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <signal.h>
#include <string.h>
#include "config.h"
#include "type.h"

static int opt_clear = 1,
       opt_image_info = 0;


static inline void do_rotate(struct image *i, int rot)
{
	if(rot)
	{
		unsigned char *image, *alpha = NULL;
		int t;
		
		image = rotate(i->rgb, i->width, i->height, rot);
		if(i->alpha)
			alpha = alpha_rotate(i->alpha, i->width, i->height, rot);
		if(i->do_free)
		{
			free(i->alpha);
			free(i->rgb);
		}
		
		i->rgb = image;
		i->alpha = alpha;
		i->do_free = 1;
		
		if(rot & 1)
		{
			t = i->width;
			i->width = i->height;
			i->height = t;
		}
	}
}


static inline void do_fit_to_screen(struct image *i, int screen_width, int screen_height, int ignoreaspect, int cal)
{
	if((i->width > screen_width) || (i->height > screen_height))
	{
		unsigned char * new_image, * new_alpha = NULL;
		int nx_size = i->width, ny_size = i->height;
		
		if(ignoreaspect)
		{
			if(i->width > screen_width)
				nx_size = screen_width;
			if(i->height > screen_height)
				ny_size = screen_height;
		}
		else
		{
			if((i->height * screen_width / i->width) <= screen_height)
			{
				nx_size = screen_width;
				ny_size = i->height * screen_width / i->width;
			}
			else
			{
				nx_size = i->width * screen_height / i->height;
				ny_size = screen_height;
			}
		}
		
		if(cal)
			new_image = color_average_resize(i->rgb, i->width, i->height, nx_size, ny_size);
		else
			new_image = simple_resize(i->rgb, i->width, i->height, nx_size, ny_size);
		
		if(i->alpha)
			new_alpha = alpha_resize(i->alpha, i->width, i->height, nx_size, ny_size);
		
		if(i->do_free)
		{
			free(i->alpha);
			free(i->rgb);
		}
		
		i->rgb = new_image;
		i->alpha = new_alpha;
		i->do_free = 1;
		i->width = nx_size;
		i->height = ny_size;
	}
}

static inline void do_enlarge(struct image *i, int screen_width, int screen_height, int ignoreaspect)
{
	if(((i->width > screen_width) || (i->height > screen_height)) && (!ignoreaspect))
		return;
	if((i->width < screen_width) || (i->height < screen_height))
	{
		int xsize = i->width, ysize = i->height;
		unsigned char * image, * alpha = NULL;
		
		if(ignoreaspect)
		{
			if(i->width < screen_width)
				xsize = screen_width;
			if(i->height < screen_height)
				ysize = screen_height;
			
			goto have_sizes;
		}
		
		if((i->height * screen_width / i->width) <= screen_height)
		{
			xsize = screen_width;
			ysize = i->height * screen_width / i->width;
			goto have_sizes;
		}
		
		if((i->width * screen_height / i->height) <= screen_width)
		{
			xsize = i->width * screen_height / i->height;
			ysize = screen_height;
			goto have_sizes;
		}
		return;
have_sizes:  
		image = simple_resize(i->rgb, i->width, i->height, xsize, ysize);
		if(i->alpha)
			alpha = alpha_resize(i->alpha, i->width, i->height, xsize, ysize);
		
		if(i->do_free)
		{
			free(i->alpha);
			free(i->rgb);
		}
		
		i->rgb = image;
		i->alpha = alpha;
		i->do_free = 1;
		i->width = xsize;
		i->height = ysize;
	}
}

/*********-------------------------------********-------------------***************************/
/*********-------------------------------********-------------------***************************/

int judge_image(char *filename)
{
	int (*load)(char *, unsigned char *, unsigned char **, int, int);

	unsigned char * image = NULL;
	unsigned char * alpha = NULL;
	
	int x_size, y_size, screen_width, screen_height;
	int x_pan, y_pan, x_offs, y_offs, refresh = 1, c, ret = 1;
	int retransform = 1;
	int transform_stretch = 0,transform_enlarge = 0,transform_cal = 2,
	    transform_iaspect = 0,transform_rotation = 0;
	
	struct image i={};
	
	if(ym_bmp_id(filename))//判断图片是否为bmp格式，返回值为1则是
	{
		if(ym_bmp_getsize(filename, &x_size, &y_size) == YM_ERROR_OK)
		{
			load = ym_bmp_load;
			goto identified;
		}
	}
		
	else if(ym_gif_id(filename))//判断图片是否为gif格式，返回值为1则是
	{
		if(ym_gif_getsize(filename, &x_size, &y_size) == YM_ERROR_OK)
		{
			load = ym_gif_load;
			goto identified;
		}
	}

	else if(ym_jpeg_id(filename))//判断图片是否为jpeg格式，返回值为1则是
	{
		if(ym_jpeg_getsize(filename, &x_size, &y_size) == YM_ERROR_OK)
		{
			load = ym_jpeg_load;
			goto identified;
		}
	}

	else if(ym_png_id(filename))//判断图片是否为png格式,返回值为1则是
	{
		if(ym_png_getsize(filename, &x_size, &y_size) == YM_ERROR_OK)
		{
			load = ym_png_load;
			goto identified;
		}
	}

  YM_ERR( "%s: Unable to access file or file format unknown !\n",filename);
	return(1);
/**********************************************************/

identified:

	if(!(image = (unsigned char*) malloc(x_size * y_size * 3)))
	{
		YM_ERR( "%s: Out of memory!\n",filename);
		goto error_mem;
	}
	
	if(load(filename, image, &alpha, x_size, y_size) != YM_ERROR_OK)
	{
		YM_ERR( "%s: Image data is corrupt !\n",filename);
		goto error_mem;
	}
	

	getCurrentRes(&screen_width, &screen_height);
	i.do_free = 0;
	
	
	while(1)//此处设置一个一个无限循环体
	{
		if(retransform)//retransform开始
		{
			if(i.do_free)
			{
				free(i.rgb);
				free(i.alpha);
			}
			i.width = x_size;
			i.height = y_size;
			i.rgb = image;
			i.alpha = alpha;
			i.do_free = 0;
	
	
			if(transform_rotation)
				do_rotate(&i, transform_rotation);
				
			if(transform_stretch)
				do_fit_to_screen(&i, screen_width, screen_height, transform_iaspect, transform_cal);
	
			if(transform_enlarge)
				do_enlarge(&i, screen_width, screen_height, transform_iaspect);

			x_pan = y_pan = 0;
			refresh = 1; 
			retransform = 0;
			if(opt_clear)
			{
				printf("\033[H\033[J");
				fflush(stdout);  //fflush(stdout)刷新标准输出缓冲区，把输出缓冲区里的东西打印到标准输出设备上
			}
			if(opt_image_info)
				YM_INFO("\033[1;40;32m%s%s%s\n%s%d x %d\n\033[0m","YaoMing - The Framebuffer Viewer\n", "filename:  ",filename,"size:  ",x_size, y_size); 
		}//if(retransform)结束
		if(refresh)
		{ 
			if(i.width < screen_width)
				x_offs = (screen_width - i.width) / 2;
			else
				x_offs = 0;
			
			if(i.height < screen_height)
				y_offs = (screen_height - i.height) / 2;
			else
				y_offs = 0;
			
			showimage(i.rgb, i.alpha, i.width, i.height, x_pan, y_pan, x_offs, y_offs);
			//显示图片
			
			refresh = 0;//transform_rotation=0;transform_stretch=0;transform_enlarge=0;
		}

		c = getchar();//从终端获取一个命令字符
		if (c=='\n') continue;//中断本次循环，如果是break则中断整个循环

		switch(c)
		{
			case 'q':
				goto done;
			//case EOF:
			case 'h':
				ret = 0;
				goto done;
    	case 'i':
      {  if(opt_image_info==0)
          opt_image_info=1;
         else opt_image_info=0;
          retransform=1;
        break;
      }
      case 'e':
      {  if(transform_enlarge==0 )
            transform_enlarge=1;
         else
            transform_enlarge=0;
         transform_iaspect=1;
         transform_cal=0;
         retransform=1;
         break;
      }
			case 'r':
      {  if(transform_stretch==0)
          transform_stretch=1;
         else transform_stretch=0;
         transform_iaspect=1;
         retransform=1;
        break;
      }
			case '1'://transform_rotation是旋转参数，=1右旋90，=2旋转180，=3左旋90
				transform_rotation -= 1;
				if(transform_rotation < 0)
					transform_rotation += 4;
				retransform = 1;
				break;
			case '2':
        transform_rotation += 2;
				if(transform_rotation > 3)
					transform_rotation -= 4;
				retransform = 1;
				break;
			case '3':
				transform_rotation += 1;
				if(transform_rotation > 3)
					transform_rotation -= 4;
				retransform = 1;
				break;	
			default :
			  ret = 2;
	      goto done;
				
		}
		
 }

done://如果opt_clear=1，则清屏
	if(opt_clear)
	{
				printf("\033[H\033[J");
				fflush(stdout);
	}

error_mem:
	free(image);
	if(i.do_free)
	{
		free(i.rgb);
		free(i.alpha);
	}
	return(ret);

}

/**********************************************************/
/**********************************************************/


