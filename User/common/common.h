#ifndef __common_h__
#define __common_h__

#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>

//=================================================

void  memREset(unsigned char *buf,unsigned char setb,unsigned char num);
void	str_cat(unsigned char	*need_cat,const char *src_cat);
unsigned char  memCpare(unsigned char *buf1,unsigned char *buf2,unsigned char num);
unsigned char	str_cmp(unsigned char	*buf1,const char *buf2);
void  memcopy(unsigned char *buf1,unsigned char *srcbuf,unsigned int num);
void	str_cpy(unsigned char	*buf1,unsigned char	*srcbuf);
unsigned char INT2Sry(long n, unsigned char *s);////将整数nn转为字符数组ss//
unsigned int str_len(unsigned char	*str);
void  Int2ASCII(unsigned char *ascbuf,unsigned long dataInt,unsigned char  num);
unsigned long  Nstr2Long(unsigned char  *str,unsigned char pp);
unsigned char Do_XOR(unsigned char *inbuf,unsigned int len);
unsigned char	ASCII2HEX(unsigned char * ascbuf, unsigned char len ,unsigned char * rehexbuf);
unsigned int	HEX2ASCII(unsigned char * hexbuf, unsigned int num ,unsigned char * ascbuf);
void FormatRunTime2(unsigned char *nowtime);
unsigned char RCVAtoI(unsigned char *str);
unsigned char Int_hasbits(unsigned long n);
unsigned char Buffercmp(unsigned char *buffer1,unsigned char *buffer2,unsigned char len);
unsigned char HexToBcd(unsigned char Value);
unsigned char BcdToHex(unsigned char Value);
#endif
