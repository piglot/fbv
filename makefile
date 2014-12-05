# Copyright (c) 2012
# All rights reserved.
# authors:   yaoming
# date:      2012-07-07

MAIN = ym 
OBJS = main.o  judge_image.o showimage.o transforms.o  jpeg.o gif.o bmp.o png.o
#OBJS = main.o  judge_image.o showimage.o transforms.o  jpeg.o gif.o bmp.o

CC = gcc
#CC = /home/ema/arm-2009q1/bin/arm-none-linux-gnueabi-gcc
#CC = /opt/crosstool/gcc-3.4.5-glibc-2.3.6/arm-davinci-linux/bin/arm-davinci-linux-gcc

INCDIRS = -I.

LDFLAGS = -lm 

CFLAGS = -Wall -O2 -g $(INCDIRS)

CFLAGS += -I/usr/include/

LIBSLINK = -lpthread 
LIBSLINK += -L.  -ljpeg -lgif -lpng -lz -lm
#LIBSLINK += -L.  -ljpeg -lgif 

DEBUG_FLAG =
.SUFFIXES= .o .c


$(MAIN): ${OBJS}
	${CC} -o $@ $(CFLAGS) ${DEBUG_FLAG} ${OBJS} $(LIBSLINK)

.o:
	$(CC) -g $(CFLAGS) -c $<

clean:
	/bin/rm -f *.o
	/bin/rm -f ym
