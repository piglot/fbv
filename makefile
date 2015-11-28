# Copyright (c) 2012
# All rights reserved.
# authors:   yaoming
# date:      2012-07-07

MAIN = fbv 
OBJS = main.o  judge_image.o showimage.o transforms.o  jpeg.o gif.o bmp.o png.o

CC = gcc
#CC = /home/ema/arm-2009q1/bin/arm-none-linux-gnueabi-gcc

INCDIRS += -I/usr/include/

CFLAGS = -Wall -O2 -g $(INCDIRS)


LIBSLINK += -L.  -ljpeg -lgif -lpng -lz -lm

DEBUG_FLAG =
.SUFFIXES= .o .c


$(MAIN): ${OBJS}
	${CC} -o $@ $(CFLAGS) ${DEBUG_FLAG} ${OBJS} $(LIBSLINK)

.o:
	$(CC) -g $(CFLAGS) -c $<

clean:
	/bin/rm -f *.o
	/bin/rm -f fbv
