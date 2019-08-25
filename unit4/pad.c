#include <stdio.h>
#include <stdlib.h>

int main(int argc,char **argv)
{
  if (argc!=2) {
    fprintf(stderr,"usage: pad <file>\n");
    exit(-1);
  }
  FILE *f=fopen(argv[1],"a");
  unsigned long offset=fseek(f,0,SEEK_END);
  offset =ftell(f);
  printf("File is currently %d bytes.\n",offset);
  for(;offset<128*1024;offset++) fputc(0x00,f);
  fclose(f);
}
