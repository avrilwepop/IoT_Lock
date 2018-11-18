#ifndef __crcencoder_h__
#define __crcencoder_h__

#include    <stdio.h>
#include    <string.h>


//=================================================

unsigned char Do_CRC8( unsigned char crc , unsigned char x ) ;
unsigned char MakeCRC8( unsigned char* dataToCRC,unsigned int leth ) ;
unsigned char Rand1Byte(unsigned int tp);
unsigned char	JM_Handler(unsigned char *jmbuf,unsigned char jiam_len);
unsigned char JaM_Handler(unsigned char *jaminbuf,unsigned char pdlen,unsigned int sjtk);
unsigned int MakeCRC16(unsigned char *message, unsigned int len);

#endif
