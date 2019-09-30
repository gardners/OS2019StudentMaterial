#include <stdio.h>
#include <stdlib.h>

int main(int argc,char **argv)
{
  FILE *out=fopen("packed.bin","w");
  int offset=0;


  if (argc==1) {
    fprintf(stderr,"usage: pack <file ...>\n");
    exit(-1);
  }
  for(int i=1;i<argc;i++) {
    FILE *f=fopen(argv[i],"r");
    unsigned char buffer[128*1024];
    unsigned long length=fread(buffer,1,128*1024,f);
    printf("File '%s' is %ld bytes long\n",argv[i],length);
    fclose(f);

    // Write out header
    // consists of program name and length and offset to next
    // header

    unsigned char header[32];
    bcopy(argv[i],&header[0],16);
    // Convert program names to screen codes to match KickC's behaviour
    for(int i=0;i<16;i++) {
      if (header[i]>='A'&&header[i]<='Z') header[i]-='A'-1;
      if (header[i]>='a'&&header[i]<='z') header[i]-='a'-1;
    }
    header[16]=length&0xff;
    header[17]=(length>>8)&0xff;
    header[18]=((offset+32+length+0x20000)>>0)&0xff;
    header[19]=((offset+32+length+0x20000)>>8)&0xff;
    header[20]=((offset+32+length+0x20000)>>16)&0xff;
    fwrite(header,1,32,out);

    // Now write out file body
    fwrite(buffer,1,length,out);

    // Advance offset
    offset+=length+32;
  }

  for(;offset<128*1024;offset++) fputc(0x00,out);
  fclose(out);
}
