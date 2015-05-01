#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "encode.h"
#include <unistd.h>
#include <sys/time.h>

#include <xil/xil.h>

unsigned char frame[262144];
main() 
{
  XilSystemState state;
  XilImage decomp_image;
  XilMemoryStorage memst;
  XilCis decompress_cis;
  int fd;
  hrtime_t before;
  int i;
  huffmanTable stdTable = {
    { 0 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 3 , 0 },
    { 1 , 2 , 0 , 3 , 4 , 5 , 6 , 7 , 8 },
  };
  
  unsigned char* dataptr;
    

  jEncoder enc = initEncoder(512,512);

  fd = open("prelude.pm",O_RDWR);
  read(fd,frame,262144);
  
  setupTables(enc,&stdTable);
  before = gethrtime();
  dataptr = (char*)getBitsPtr(enc);
  printf("%02x %02x %d\n",dataptr[55] , dataptr[56] , i);
  for(i = 0; i < 1000; i++) {
    encodeFrame(enc,(char*)frame);
    /*   printf("%02x %02x %d\n",dataptr[55] , dataptr[56] , i);*/
  }
  printf("%lld\n",(gethrtime() - before)/i);
  printf("compressed size: %d\n",getCompressedSize(enc));
  fd = open("jout",O_RDWR);
  write(fd,getBitsPtr(enc),getCompressedSize(enc));
  close(fd);
  state = xil_open();
  decompress_cis = xil_cis_create(state,"JpegLL");
  decomp_image = xil_create(state,512,512,1,XIL_BYTE);
  
  xil_cis_put_bits_ptr(decompress_cis,getCompressedSize(enc),1,getBitsPtr(enc),NULL);
  xil_decompress(decompress_cis,decomp_image);
  
  xil_export(decomp_image);
  xil_get_memory_storage(decomp_image,&memst);
  
  for(i = 0 ; i < 20; i++)
    if(memst.byte.data[i] != frame[i])
      printf("%02x / %02x at %d\n",memst.byte.data[i],frame[i],i);
}

    
