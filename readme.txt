�����򿪷�������
Pc������ϵͳΪ��Linux ������Ubuntu 8.04���ں˰汾Ϊlinux2.6.24��
PC�˿�������Ϊ�� vim��gcc
ϵͳ�������ԣ�Linux C
������빤��Ϊ��arm-none-linux-gnueabi-gcc
�����廷��Ϊ��TOP35X������
�����������CPU��ARM11��RAM=256MB��ROM=2GB������оƬ����S3C6410��
�������룺���ն�������make����������Ϊym�Ŀ�ִ�г��� 
������룺��1������ͼƬ��̬��ѹ���ļ�����ѹ��������Ϊ��
              tar �Cxvf zlib-1.2.3.tar.gz
 		  �޸�����makefile.linux��
		  cc=/home/ema/arm-2009q1/bin/arm-none-linux-gnueabi-gcc
		  prefix =/usr/local
	        �޸ĺ󱣴�Ϊmakefile��
	   ��2���������ɿ������ڽ���������ӵľ�̬�⡣����Ϊ��
		  ./configure
		  make
		  make install
          (3)�޸ı�������makefile�еĽ�����빤��Ϊ��
	       CC = /home/ema/arm-2009q1/bin/arm-none-linux-gnueabi-gcc
	    ��4��make�������ɿ�ִ�г���
ע����1��ͼƬ�ļ����ڵ�ǰĿ¼�£�֧��BMP,JPG,GIF,PNG����ͼƬ��
    ��2���ն������и�ʽΪ��./ym ͼƬ�ļ���������������������������
    ��3��ͼƬ��ʾ����������У�����һ���ַ������ºϷ���1,2,3��e,r,i,h,q��