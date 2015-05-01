#pragma ident "@(#) encode.h 1.3@(#) Copyright © by Konstantin Priblouda 1997 all rigths reserved"

typedef void* jEncoder;

/*
  huffman table in stabdart format
  for curiosed just look int the specs of IJG
  or pennebaker book , or in JPEG library
  */
typedef struct _huffmanTable 
{
  unsigned char bits[17];
  unsigned char huffval[256];
} huffmanTable;



/* 
   inits encoder , with specified data
   returns initialized data struct which can be reused
   for different frames setups default huffman tables
   */
jEncoder initEncoder(int width, /* width of image */
		     int height /* height of image */
		     );

/* 
   setup encoding tables according specified huffman tables
   */
void setupTables(jEncoder enc,
		 huffmanTable* htbl);

/*
  encodes one frame of data , returns amount of bytes coded 
  gets encoder struct as parameter and pointer to the frame
  data  , it has to be in pixel sequential format
  and of specified size
  */
void  encodeFrame(jEncoder enc,char* frameData);

/* 
   returns pointer to the compressed data
   */
void* getBitsPtr(jEncoder enc);

/*
  returns size of compressed data held
  */
int getCompressedSize(jEncoder enc);

/* 
   destroys encoder and free allocated data 
   */
void destroyEncoder(jEncoder enc);
