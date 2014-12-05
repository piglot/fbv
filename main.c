
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include "config.h"
#include "type.h"

void help(char *name)
{
	YM_INFO("\n"
	     "Corret usage: %s  imagename\n\n"
		   "Available options after showing image:\n"
       " q            : Quit \n"
       " h            : Quit and show information of help\n" 
       " i            : Show or not show image information\n"
       " e            : Enlarge the image to fit the whole screen or recover\n"
       " r            : Reducee the image to fit the whole screen or recover\n"
		   " 1            : Rotate the image 90 degrees left\n"
		   " 2            : Rotate the image 180 degrees \n"
		   " 3            : Rotate the image 90 degrees right\n"
		   "Copyright (C) 2012 - 2013 FZXY, Yao Ming.\n", name);
}

/*************************************************************/
/*************************************************************************/

int main(int argc, char **argv)
{
	int  i=1;
	if(argc != 2)
	{
		YM_ERR( "Error: The number of arguments required is wrong.\n");
		help(argv[0]);
		return(1);
	}

    if(argv[i])
	 {		
	  int r=judge_image(argv[i]);//通用接函数判断口.正常执行完return（1），获取命令字符q后return（0）
	  if(r<1) 
      help(argv[0]);
    else if(r>1)
      YM_ERR("ERR: Option input illegal !\n");   
	 }

	return(0);	
}
