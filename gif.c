
#include "config.h"
#ifdef YM_SUPPORT_GIF
#include "type.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <setjmp.h>
#include <gif_lib.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define min(a,b) ((a) < (b) ? (a) : (b))
#define gflush return(YM_ERROR_FILE);
#define grflush { DGifCloseFile(gft); return(YM_ERROR_FORMAT); }
#define mgrflush { free(lb); free(slb); DGifCloseFile(gft); return(YM_ERROR_FORMAT); }
#define agflush return(YM_ERROR_FORMAT);
#define agrflush { DGifCloseFile(gft); return(YM_ERROR_FORMAT); }


int ym_gif_id(char *name)
{
    int fd;
    char id[4];
    fd=open(name,O_RDONLY); if(fd==-1) return(0);
    read(fd,id,4);
    close(fd);
    if(id[0]=='G' && id[1]=='I' && id[2]=='F') return(1);//GIF署名用来确认一个文件是否是GIF格式的文件，这一部分由三个字符组成："GIF";
    return(0);
}

inline void ym_rend_gif_decodecolormap(unsigned char *cmb,unsigned char *rgbb,ColorMapObject *cm,int s,int l, int transparency)
{
    GifColorType *cmentry;
    int i;
    for(i=0;i<l;i++)
    {
	cmentry=&cm->Colors[cmb[i]];
	*(rgbb++)=cmentry->Red;
	*(rgbb++)=cmentry->Green;
	*(rgbb++)=cmentry->Blue;
    }
}



int ym_gif_load(char *name,unsigned char *buffer, unsigned char ** alpha, int x,int y)
{
  int in_nextrow[4]={8,8,4,2};   //interlaced jump to the row current+in_nextrow
  int in_beginrow[4]={0,4,2,1};  //begin pass j from that row number
  int transparency=-1;  //-1 means not transparency present
    int px,py,i,ibxs;
    int j;
    char *fbptr;
    char *lb;
    char *slb;
    GifFileType *gft;
    GifByteType *extension;
    int extcode;
    GifRecordType rt;
    ColorMapObject *cmap;
    int cmaps;

    gft=DGifOpenFileName(name);
    if(gft==NULL){printf("err5\n"); gflush;} //////////
    do
    {
	if(DGifGetRecordType(gft,&rt) == GIF_ERROR) grflush;
	switch(rt)
	{
	    case IMAGE_DESC_RECORD_TYPE:
		if(DGifGetImageDesc(gft)==GIF_ERROR) grflush;
		px=gft->Image.Width;
		py=gft->Image.Height;
		lb=(char*)malloc(px*3);
		slb=(char*) malloc(px);
//  printf("reading...\n");
		if(lb!=NULL && slb!=NULL)
		{
			unsigned char *alphaptr = NULL;
			
		    cmap=(gft->Image.ColorMap ? gft->Image.ColorMap : gft->SColorMap);
		    cmaps=cmap->ColorCount;

		    ibxs=ibxs*3;
		    fbptr=(char*)buffer;
			
			if(transparency != -1)
			{
				alphaptr = malloc(px * py);
				*alpha = alphaptr;
			}
			
		    if(!(gft->Image.Interlace))
		    {
			for(i=0;i<py;i++,fbptr+=px*3)
			{
				int j;
			    if(DGifGetLine(gft,(GifPixelType*)slb,px)==GIF_ERROR) mgrflush;
			    ym_rend_gif_decodecolormap((unsigned char*)slb,(unsigned char*)lb,cmap,cmaps,px,transparency);
			    memcpy(fbptr,lb,px*3);
				if(alphaptr)
					for(j = 0; j<px; j++) *(alphaptr++) = (((unsigned char*) slb)[j] == transparency) ? 0x00 : 0xff;
        		}
                    }
                    else
		    {
				unsigned char * aptr = NULL;
				
	               for(j=0;j<4;j++)
	               {
						int k;
				        if(alphaptr)
							aptr = alphaptr + (in_beginrow[j] * px);
							
					    fbptr=(char*)buffer + (in_beginrow[j] * px * 3);

					    for(i = in_beginrow[j]; i<py; i += in_nextrow[j], fbptr += px * 3 * in_nextrow[j], aptr += px * in_nextrow[j])
			    {
				if(DGifGetLine(gft,(GifPixelType*)slb,px)==GIF_ERROR) mgrflush; /////////////
				ym_rend_gif_decodecolormap((unsigned char*)slb,(unsigned char*)lb,cmap,cmaps,px,transparency);
				memcpy(fbptr,lb,px*3);
				if(alphaptr)
					for(k = 0; k<px; k++) aptr[k] = (((unsigned char*) slb)[k] == transparency) ? 0x00 : 0xff;
          		    }
			}
		    }
		}
		if(lb) free(lb);
		if(slb) free(slb);
                break;
	    case EXTENSION_RECORD_TYPE:
		if(DGifGetExtension(gft,&extcode,&extension)==GIF_ERROR) grflush; //////////
		if(extcode==0xf9) //look image transparency in graph ctr extension
		{
			if(extension[1] & 1)
			{
				transparency = extension[4];

			}
//		    tran_off=(int)*extension;
//		    transparency=(int)*(extension+tran_off);
//			printf("transparency: %d\n", transparency);
                }
		while(extension!=NULL)
		    if(DGifGetExtensionNext(gft,&extension) == GIF_ERROR) grflush
		break;
	    default:
		break;
	}
    }
    while( rt!= TERMINATE_RECORD_TYPE );
    DGifCloseFile(gft);
    return(YM_ERROR_OK);
}



int ym_gif_getsize(char *name,int *x,int *y)
{
    int px,py;
    GifFileType *gft;
    GifByteType *extension;
    int extcode;
    GifRecordType rt;

    gft=DGifOpenFileName(name);
    if(gft==NULL) gflush;
    do
    {
	if(DGifGetRecordType(gft,&rt) == GIF_ERROR) grflush;
	switch(rt)
	{
	    case IMAGE_DESC_RECORD_TYPE:

		if(DGifGetImageDesc(gft)==GIF_ERROR) grflush;
		px=gft->Image.Width;
		py=gft->Image.Height;
		*x=px; *y=py;
		DGifCloseFile(gft);
		return(YM_ERROR_OK);
		break;
	    case EXTENSION_RECORD_TYPE:
		if(DGifGetExtension(gft,&extcode,&extension)==GIF_ERROR) grflush;
		while(extension!=NULL)
		    if(DGifGetExtensionNext(gft,&extension)==GIF_ERROR) grflush;
		break;
	    default:
		break;
	}  
    }
    while( rt!= TERMINATE_RECORD_TYPE );
    DGifCloseFile(gft);
    return(YM_ERROR_FORMAT);
}
#endif
