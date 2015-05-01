#pragma ident "@(#) encodeP.h 1.2@(#) Copyright © by Konstantin Priblouda 1997 all rigths reserved"

/*
  private Include file for the loseless jpeg encoder
  */

/* 
   already encoded difference value along with
   its size
   */
typedef struct _huffmanDescriptor 
{
  long huffmanCode;
  long huffmanSize;
} huffmanDescriptor;



/* 
   data struct for the encoder 
   there is one big buffer , designed to hold BOTH
   header and encoded data , there will be buffer
   big enough to keep all compressed data along with a header
   */
typedef struct _encoderData 
{
  /* encoded values indexed with difference value */
  huffmanDescriptor huffDesc[512];
  int xSize; /* sizes of image in question */
  int ySize; 
  char *dataBuf; /* space reserved for encoded data along with header */
  int dataBufSize; /* size of this buffer */
  
  /* huffman table in DHT format */
  huffmanTable* htbl;
  
  char* headerInfo; /*pointer to header part  of compressed frame */
  int headerSize; /* size of header part of compressed frame */

  char* scanData; /* pointer to the compressed scan data */
  int scanSize; /* size of the compressed scan */

  int frameSize; /* size of a frame coded */
  
} encoderData;


