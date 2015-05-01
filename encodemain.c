#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "encode.h"
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dmedia/dmedia.h>



unsigned char frame[262144];
main() 
{
int fd;
  unsigned char* dataptr;
   huffmanTable stdTable = {
    { 0 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 3 , 0 },
    { 1 , 2 , 0 , 3 , 4 , 5 , 6 , 7 , 8 },
  };   
int i;

unsigned long long before , after;
  jEncoder enc = initEncoder(512,512);

  fd = open("prelude.pm",O_RDWR);
  read(fd,frame,262144);
  
  setupTables(enc,&stdTable);
  dmGetUST(&before);
  dataptr = (char*)getBitsPtr(enc);
  printf("%02x %02x %d\n",dataptr[55] , dataptr[56] , i);
  for(i = 0; i < 1000; i++) {
    encodeFrame(enc,(char*)frame);
    /*   printf("%02x %02x %d\n",dataptr[55] , dataptr[56] , i);*/
  }
  dmGetUST(&after);
  printf("%lld\n",(after - before)/i);
  printf("compressed size: %d\n",getCompressedSize(enc));
  fd = open("jout",O_WRONLY|O_CREAT,0666);
  write(fd,getBitsPtr(enc),getCompressedSize(enc));
  close(fd);

}

    
