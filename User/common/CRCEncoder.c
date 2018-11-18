#include "crcencoder.h"

unsigned char Near_bg_data;

const unsigned int  crc16_ccitt_table2[256]={               // 1021CRC余式表 //
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
  };
	/*unsigned short crc16_ccitt_table[256] =   
	{   
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,   
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,   
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,   
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,   
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,   
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,   
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,   
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,   
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,   
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,   
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,   
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,   
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,   
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,   
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,   
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,   
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,   
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,   
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,   
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,   
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,   
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,   
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,   
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,   
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,   
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,   
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,   
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,   
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,   
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,   
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,   
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78  
	}; 
	*/
//============================================================//
unsigned char const THJM_bg[256]=
{
125, 67 , 229, 244, 129, 91 , 204, 132, 55 , 216,
163, 104, 224, 228, 4  , 94 , 130, 251, 167, 50 ,
53 , 9  , 236, 10 , 172, 13 , 247, 27 , 0  , 15 ,
241, 103, 165, 189, 222, 107, 148, 190, 243, 168,
21 , 25 , 221, 102, 166, 237, 45 , 29 , 70 , 134,
32 , 73 , 33 , 171, 34 , 113, 35 , 233, 38 , 64 ,
48 , 238, 49 , 6  , 54 , 205, 110, 56 , 81 , 30 ,
93 , 174, 59 , 232, 136, 115, 61 , 86 , 155, 44 ,
17 , 200, 36 , 18 , 74 , 253, 19 , 20 , 156, 235,
187, 76 , 227, 191, 151, 24 , 141, 31 , 79 , 120,
40 , 57 , 42 , 118, 234, 43 , 182, 196, 80 , 114,
239, 194, 135, 14 , 75 , 78 , 220, 246, 47 , 108,
41 , 1  , 207, 193, 3  , 188, 5  , 65 , 206, 8  ,
173, 62 , 83 , 180, 84 , 28 , 245, 126, 87 , 128,
7  , 90 , 51 , 92 , 218, 153, 201, 154, 95 , 219,
2  , 96 , 99 , 140, 100, 254, 158, 230, 101, 143,
116, 82 , 117, 22 , 203, 121, 16 , 97 , 123, 124,
164, 240, 66 , 26 , 68 , 231, 105, 69 , 127, 71 ,
223, 133, 170, 85 , 58 , 137, 215, 157, 138, 139,
109, 217, 186, 242, 192, 112, 197, 146, 11 , 248,
147, 199, 149, 209, 37 , 119, 249, 46 , 152, 89 ,
210, 98 , 39 , 60 , 159, 213, 214, 160, 122, 179,
23 , 195, 202, 142, 176, 144, 183, 145, 88 , 208,
169, 150, 131, 111, 175, 250, 77 , 177, 63 , 178,
198, 162, 161, 226, 12 , 255,
211, 181, 72 , 52 , 212, 184, 225, 185, 106, 252
};

/**
 * CRC 校验编码器
 * @author
 * @version 2.0
 */
unsigned char const  dscrc_table[] = 
{   0 , 
    94 , 188 , 226 , 97 , 
    63 , 221 , 131 , 194 , 
    156 ,  126 ,  32 ,  163 , 
    253 ,  31 ,  65 ,  157 , 
    195 ,  33 ,  127 ,  252 , 
    162 ,  64 ,  30 ,  95 , 
    1 ,  227 ,  189 ,  62 , 
    96 ,  130 ,  220 ,  35 , 
    125 ,  159 ,  193 ,  66 , 
    28 ,  254 ,  160 ,  225 , 
    191 ,  93 ,  3 ,  128 , 
    222 ,  60 ,  98 ,  190 , 
    224 ,  2 ,  92 ,  223 , 
    129 ,  99 ,  61 ,  124 , 
    34 ,  192 ,  158 ,  29 , 
    67 ,  161 ,  255 ,  70 , 
    24 ,  250 ,  164 ,  39 , 
    121 ,  155 ,  197 ,  132 , 
    218 ,  56 ,  102 ,  229 , 
    187 ,  89 ,  7 ,  219 , 
    133 ,  103 ,  57 ,  186 , 
    228 ,  6 ,  88 ,  25 , 
    71 ,  165 ,  251 ,  120 , 
    38 ,  196 ,  154 ,  101 , 
    59 ,  217 ,  135 ,  4 , 
    90 ,  184 ,  230 ,  167 , 
    249 ,  27 ,  69 ,  198 , 
    152 ,  122 ,  36 ,  248 , 
    166 ,  68 ,  26 ,  153 , 
    199 ,  37 ,  123 ,  58 , 
    100 ,  134 ,  216 ,  91 , 
    5 ,  231 ,  185 ,  140 , 
    210 ,  48 ,  110 ,  237 , 
    179 ,  81 ,  15 ,  78 , 
    16 ,  242 ,  172 ,  47 , 
    113 ,  147 ,  205 ,  17 , 
    79 ,  173 ,  243 ,  112 , 
    46 ,  204 ,  146 ,  211 , 
    141 ,  111 ,  49 ,  178 , 
    236 ,  14 ,  80 ,  175 , 
    241 ,  19 ,  77 ,  206 , 
    144 ,  114 ,  44 ,  109 , 
    51 ,  209 ,  143 ,  12 , 
    82 ,  176 ,  238 ,  50 , 
    108 ,  142 ,  208 ,  83 , 
    13 ,  239 ,  177 ,  240 , 
    174 ,  76 ,  18 ,  145 , 
    207 ,  45 ,  115 ,  202 , 
    148 ,  118 ,  40 ,  171 , 
    245 ,  23 ,  73 ,  8 , 
    86 ,  180 ,  234 ,  105 , 
    55 ,  213 ,  139 ,  87 , 
    9 ,  235 ,  181 ,  54 , 
    104 ,  138 ,  212 ,  149 , 
    203 ,  41 ,  119 ,  244 , 
    170 ,  72 ,  22 ,  233 , 
    183 ,  85 ,  11 ,  136 , 
    214 ,  52 ,  106 ,  43 , 
    117 ,  151 ,  201 ,  74 , 
    20 ,  246 ,  168 ,  116 , 
    42 ,  200 ,  150 ,  21 , 
    75 ,  169 ,  247 ,  182 , 
    232 ,  10 ,  84 ,  215 , 
    137 ,  107 , 
    53 
};

   /**
    * 根据码表计算
    * @param crc
    * @param x
    * @return
    */
   unsigned char Do_CRC8( unsigned char crc , unsigned char x ) 
   {

      int index = crc ^ x;

      if ( index < 0 ) 
      {
	     index = index + 256;
      }

      return (dscrc_table[ index ]);
   }

   /**
    * 计算CRC机校检码
    * @param dataToCRC 待计算的数据
    * @return
    */
   unsigned char MakeCRC8( unsigned char* dataToCRC,unsigned int leth ) 
   {

      unsigned char crc = 0;
      unsigned int  i;
			 
      for (  i = 0; i < leth; i++ ) 
      {
	      crc = Do_CRC8( crc , dataToCRC[ i ] );
      }
			 
      return crc;
   }


//=================================================//
unsigned char Rand1Byte(unsigned int tp)
{
	unsigned char result,linshibuf[4];
	
	linshibuf[0] = ((tp>>12)&0xf0);
	linshibuf[1] = ((tp>>4)&0xff);
	linshibuf[2] = ((tp<<4)&0xf0);
	linshibuf[3] = (linshibuf[0] | linshibuf[2]);
	result = (linshibuf[1]^linshibuf[3]);
	return (result);
}

//=======================================================================//
unsigned char	JM_Handler(unsigned char *jmbuf,unsigned char jiam_len)
{
	unsigned char i,j;
	unsigned char pdlen;
	unsigned int  tmpint;

	pdlen=jiam_len-3;
  //-----------------------------------
	tmpint = jmbuf[pdlen];
	tmpint = ((tmpint<<8)|jmbuf[pdlen+1]);
	j = Rand1Byte(tmpint);
  //-----------------------------------
	if(jmbuf[pdlen+2]==1) {j+=10;}
	//-----------------------------------
	for(i=0;i<pdlen;i++)
	{
		if(j>254)	{j=0;}
		jmbuf[i] ^=THJM_bg[j];
		j++;
	}
  
	return (pdlen);
}
 
//===================================================================//
unsigned char JaM_Handler(unsigned char *jaminbuf,unsigned char pdlen,unsigned int sjtk)
{
	unsigned char i,j,a1flag;
	
	//-----------------------------------
	j = Rand1Byte(sjtk);
	/*if(j ==Near_bg_data)
	{
		j+=10;
		Near_bg_data=j;
		a1flag=1;
	}
	else
	*/
	{ Near_bg_data=j; a1flag=0;}
	//-----------------------------------
	for(i=0;i<pdlen;i++)
	{
		if(j>254){j=0;}
		jaminbuf[i]^=THJM_bg[j]; 
		j++;
	}
	//-----------------------------------
	jaminbuf[i] = (unsigned char)(sjtk>>8);
	jaminbuf[i+1] = (unsigned char)(sjtk&0x00ff);		
	jaminbuf[i+2]= a1flag;

	return (pdlen+3);
}

//=================================================================================//
/*unsigned int MakeCRC16(unsigned char *message, unsigned int len)    
{   
  unsigned int crc_reg = 0xffff;    
           
  while (len--)    
    {  crc_reg = (crc_reg >> 8) ^ crc16_ccitt_table[(crc_reg ^ *message++) & 0xff];   }
         
  return crc_reg;   
} */  

unsigned int MakeCRC16(unsigned char *message, unsigned int len) 
{
    unsigned short int crc;
    unsigned char da;
    crc=0XFFFF;
    while(len--!=0) 
    {
        da=(unsigned short)crc>>8;    // 以8位二进制数的形式暂存CRC的高8位 //
        crc<<=8;                      // 左移8位，相当于CRC的低8位乘以     //
        crc^=crc16_ccitt_table2[da^*message];     // 高8位和当前字节相加后再查表求CRC,再加上以前的CRC //
        message++;
    }
    return(crc);
}
//===================================================================================//
