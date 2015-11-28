本程序开发环境：
Pc机操作系统为：Linux （基于Ubuntu 8.04，内核版本为linux2.6.24）
PC端开发工具为： vim，gcc
系统开发语言：Linux C
交叉编译工具为：arm-none-linux-gnueabi-gcc
开发板环境为：TOP35X开发板
开发板参数：CPU：ARM11，RAM=256MB，ROM=2GB，核心芯片三星S3C6410。
正常编译：在终端下输入make即可生成名为ym的可执行程序 
交叉编译：（1）下载图片静态库压缩文件，解压缩，命令为：
              tar –xvf zlib-1.2.3.tar.gz
 		  修改其中makefile.linux：
		  cc=/home/ema/arm-2009q1/bin/arm-none-linux-gnueabi-gcc
		  prefix =/usr/local
	        修改后保存为makefile。
	   （2）编译生成可以用于交叉编译链接的静态库。命令为：
		  ./configure
		  make
		  make install
          (3)修改本主程序makefile中的交叉编译工具为：
	       CC = /home/ema/arm-2009q1/bin/arm-none-linux-gnueabi-gcc
	    （4）make即可生成可执行程序
注：（1）图片文件放在当前目录下，支持BMP,JPG,GIF,PNG四种图片；
    （2）终端命令行格式为：./ym 图片文件名（仅允许输入这两个参数）
    （3）图片显示后进入命令行，输入一个字符，以下合法：1,2,3，e,r,i,h,q。