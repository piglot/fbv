<1>本程序开发环境：
	PC机操作系统：Linux （基于Ubuntu 14.04，Linux Kernel 3.13.8）
	系统开发语言：Linux C

	PC机编译工具： gcc
	交叉编译工具：arm-none-linux-gnueabi-gcc

	开发板：TOP35X开发板
	开发板参数：CPU=ARM11，RAM=256MB，ROM=2GB，核心芯片三星S3C6410。
<2>编译和运行
	正常编译：生成可在PC端文本模式下运行的可执行程序
		makefile中编译工具：
			CC=gcc
	 	make即可生成名为fbv的可执行程序 

	交叉编译：生成可以在Top35X开发板上运行的可执行程序
		1. 静态库的下载，修改，编译和安装：
			（1）下载jpeg, gif和png三种静态库压缩文件并解压缩。举例：
        			tar –xvf zlib-1.2.3.tar.gz
 			（2）修改解压出的文件中的makefile.linux，并保存重命名为makefile
        			cc=/home/ema/arm-2009q1/bin/arm-none-linux-gnueabi-gcc
				prefix =/usr/local
			（3）编译生成可以用于交叉编译链接的静态库。命令为：
				./configure
		 		make
		 		make install
          	2. 设置编译工具。
          		修改我的代码中的makefile：
				CC = /home/ema/arm-2009q1/bin/arm-none-linux-gnueabi-gcc
	  	3. 编译运行。
	  		make即可生成可执行程序fbv
	  	
注：（1）支持BMP,JPG,GIF,PNG四种格式的图片显示和放缩旋转等操作；
    （2）终端运行命令：./fbv 图片文件路径（仅允许输入这两个参数）
    （3）图片显示后会进入命令行请用户输入一个字符以完成相应的操作，以下为合法：1,2,3，e,r,i,h,q。
