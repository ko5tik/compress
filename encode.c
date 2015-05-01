#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "encode.h"
#include "encodeP.h"
#include "encodeScan.h"

#pragma ident "@(#) encode.c 1.4@(#) Copyright © by Konstantin Priblouda 1997 all rigths reserved"
static int bmask[] = {0x0000,
	 0x00000001, 0x00000003, 0x00000007, 0x0000000F,
	 0x0000001F, 0x0000003F, 0x0000007F, 0x000000FF,
	 0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF,
	 0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
	 0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF,
	 0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
	 0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF,
	 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF};

static int numBitsTable[256];

/* default huffman table just like one from XIL - it's far better than philips */
static huffmanTable stdTable = {
  { 0 , 0,  1 , 5 , 1 , 1 , 1 , 0 },
  { 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 }
};

/* inits encoder , setups sizes and number of bits table
   although we do not need this table in compression procedure
   we keep this from IJG code ,
   encoder data struct is allocated by the function and filled in correctly
   space to hold compressed frame is allocated also
   also setups default huffman tables
   */

jEncoder initEncoder(int width,
		     int height)
{
  int i,temp,nbits;
  encoderData* enc;
  
  /* assert that width and height are less than 65536 */
  if(width >= 65536 || height >= 65536)
    return (jEncoder)NULL;

  /* allocate space for the struct itself */
  enc = (encoderData*)malloc(sizeof(encoderData));

  if(!enc)
    return (jEncoder)NULL;

  enc->xSize = width;
  enc->ySize = height;
  
  /* init buffer for compressed data, it has to be as big as a frame , just to be sure */
  enc->dataBuf = malloc(width * height);
  if(!enc->dataBuf) {
    free(enc);
    return NULL;
  }
  
  enc->dataBufSize = width * height;
  enc->frameSize = 0;
  
  /* setup bits table */
  for (i=0; i<256; i++) {
    temp = i;
    nbits = 1;
    while (temp >>= 1) {
      nbits++;
    }
    numBitsTable[i] = nbits;
  }
  /* setup all what we need */
  setupTables((jEncoder)enc,&stdTable);
  
  /* copy header in the outptr */
  return (jEncoder)enc;
  
}

/* 
   performs comlete setup for the encoding -
   gets tables , and composes header and trailer
   to this parameters
   */

void setupTables(jEncoder ee,
		 huffmanTable* htbl) 
{
  int p, i, l, lastp, si;
  int huffsize[257];
  unsigned short huffcode[257];
  unsigned short code;
  unsigned short ehufco[256] , ehufsi[256];
  int temp , temp2,nbits;
  int length;
  int headpos;
  encoderData* enc = (encoderData*)ee;
  
  
  
  int size;
  int value, ll, ul;

  /* assert we have valid pointers */
  if(!enc) 
    return;
  if(!htbl)
    return;
  enc->htbl = htbl;
  
  /* how fix huffman tables and create descriptors */
  /*
   * Figure C.1: make table of Huffman code length for each symbol
   * Note that this is in code-length order.
   */
    p = 0;
    for (l = 1; l <= 16; l++) {
        for (i = 1; i <= (int)htbl->bits[l]; i++)
            huffsize[p++] = (char)l;
    }
    huffsize[p] = 0;
    lastp = p;
    
    /*
     * Figure C.2: generate the codes themselves
     * Note that this is in code-length order.
     */
    code = 0;
    si = huffsize[0];
    p = 0;
    while (huffsize[p]) {
      while (((int)huffsize[p]) == si) {
	huffcode[p++] = code;
	code++;
      }
      code <<= 1;
      si++;
    }
    /*
     * Figure C.3: generate encoding tables
     * These are code and size indexed by symbol value
     * Set any codeless symbols to have code length 0; this allows
     * EmitBits to detect any attempt to emit such symbols.
     */
    memset(ehufsi, 0, sizeof(ehufsi));

    for (p = 0; p < lastp; p++) {
        ehufco[htbl->huffval[p]] = huffcode[p];
        ehufsi[htbl->huffval[p]] = huffsize[p];
	/*	printf("%d %03x  %d\n",htbl->huffval[p],huffcode[p],huffsize[p]);*/
	
    }
    /* after this tables are generated , we walk through our code space
       and produce descriptors */
    for(i = -255; i < 256; i++) {
      temp = temp2 = i;
      if(temp < 0) {
	temp = -temp;
	temp2--;
      }
      nbits = 0;
      if(temp) {
	while(temp >= 256) 
	  {
	    nbits += 8;
	    temp >>= 8;
	  }
	  nbits += numBitsTable[temp&0xff];
      }
      /* compose descriptor from temp2 , nbits and huffcode */
      enc->huffDesc[ i & 0x1ff ].huffmanCode = (ehufco[nbits] << nbits) | ( temp2 & bmask[nbits]);
      enc->huffDesc[ i & 0x1ff ].huffmanSize = ehufsi[nbits] + nbits;
      
    }
    /*  for(i = 0; i < 512; i++) {
      printf("%03x %04x %d\n", i , enc->huffDesc[i].huffmanCode , enc->huffDesc[i].huffmanSize);
    }*/
    
    /* make header point to the start of allocated space */
    enc->headerInfo = enc->dataBuf;
    
    /* 
       create SOF / SOS / EOF data 
       */
    headpos = 0;

    
    enc->headerInfo[headpos++] = 0xff;
    enc->headerInfo[headpos++] = 0xd8; /* soi */

    /* now comes DHT seq */
    enc->headerInfo[headpos++] = 0xff;
    enc->headerInfo[headpos++] = 0xc4;
    /* emit huffman table*/
    length = 0;
    for(i = 1; i <= 16 ; i++)
      length += htbl->bits[i];
    
    enc->headerInfo[headpos++] = (char)(((length+19) >> 8) & 0xff);
    enc->headerInfo[headpos++] = (char)((length+19) &0xff);

    enc->headerInfo[headpos++] = 0; /* index */

    for(i = 1; i<=16;i++) 
      enc->headerInfo[headpos++] = htbl->bits[i];
    for(i = 0;i<length;i++)
     enc-> headerInfo[headpos++] = htbl->huffval[i];
    
    /* SOF */
    enc->headerInfo[headpos++] = 0xff; 
    enc->headerInfo[headpos++] = 0xc3; /* sof */
    enc->headerInfo[headpos++] = 0;
    enc->headerInfo[headpos++] = (char)(((3 * 1) + 2 + 5 + 1) & 0xff); /* length of header 1 id number of scans */
    enc->headerInfo[headpos++] = 8; /* data precision */
    enc->headerInfo[headpos++] = (char)((enc->xSize >> 8 ) &0xff);
    enc->headerInfo[headpos++] = (char)(enc->xSize &0xff); /* image width */
    enc->headerInfo[headpos++] = (char)((enc->ySize >> 8 ) &0xff);
    enc->headerInfo[headpos++] = (char)(enc->ySize &0xff); /* image height */
    enc->headerInfo[headpos++] = 1; /* number of components */
    enc->headerInfo[headpos++] = 0; /* component number 0 */
    enc->headerInfo[headpos++] = 0x11; /* both sampling factors are 1 */
    enc->headerInfo[headpos++] = 0x0; /* sincce we use loseless comporession */

    /* time for start of scan ( SOS ) */
    enc->headerInfo[headpos++] = 0xff;
    enc->headerInfo[headpos++] = 0xda; /* SOS */
    enc->headerInfo[headpos++] = 0x0;
    enc->headerInfo[headpos++] = 0x8; /* length of sos , since e have only one component */
    enc->headerInfo[headpos++] = 0x1; /* number of components */
    enc->headerInfo[headpos++] = 0x0; /* component selector */
    enc->headerInfo[headpos++] = 0x0; /* component id and dctbl */
    enc->headerInfo[headpos++] = 0x1; /* PSV */
    enc->headerInfo[headpos++] = 0; /* spectarl selection */
    enc->headerInfo[headpos++] = 0; /* point transform */
    
    /* we are ready with this ... , header id done */
    enc->headerSize = headpos;
    /* setup scan pointer to the byte just after the header */
    enc->scanData = &(enc->dataBuf[enc->headerSize]);
    enc->scanSize = 0;
    
}

/* encode one frame of data */
void encodeFrame(jEncoder ee,char* frameData) 
{
  int qq;
  int i;
  
  encoderData* enc = (encoderData*)ee;
  qq = encodeScan(frameData,enc->xSize,enc->ySize,enc->scanData,enc->huffDesc);
  enc->scanSize = qq;
  enc->frameSize = enc->headerSize + enc->scanSize;
}

/* retuns poiner to the compressed data */
void *getBitsPtr(jEncoder enc) 
{
  return (void*)(((encoderData*)enc)->dataBuf);
}

/* returns size of compressed data */
int getCompressedSize(jEncoder enc) 
{
  return ((encoderData*)enc)->frameSize;
}

void destroyEncoder(jEncoder enc) 
{
  encoderData* ee = (encoderData*)enc;
  if(ee->dataBuf)
    free(ee->dataBuf);
  free(ee);
}
