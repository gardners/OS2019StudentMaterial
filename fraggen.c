#include <stdio.h>
#include <string.h>
#include <strings.h>

/*

  lvalue=[v|p|_deref_p][d|s|c][u|s][c|z][1|2]
  rvalue=[_hi_|_lo_|]<lvalue>[[_rol_|_ror_][1-32|<lvalue>]]
  
  <lvalue>=<rvalue>
*/

#define CHOICE(N,M)   case N: snprintf(&lvalue_name_ret[ofs],1024-ofs,"%s",M"p"); ofs+=strlen(M); break
#define RCHOICE(N,M)   case N: snprintf(&rvalue_name_ret[ofs],1024-ofs,"%s",M"p"); ofs+=strlen(M); break
#define DEFCHOICE(N,WAYS) c##N=idx%WAYS; idx=idx/WAYS
  
#define LVALUE_FORMS 72
#define RVALUE_FORMS

char lvalue_name_ret[1024];
char *lvalue_name(int idx)
{
  int c1,c2,c3,c4,c5;

  DEFCHOICE(1,3);
  DEFCHOICE(2,3);
  DEFCHOICE(3,2);
  DEFCHOICE(4,2);
  DEFCHOICE(5,2);
  
  int ofs=0;
  switch(c1) {
    CHOICE(0,"v");
    CHOICE(1,"p");
    CHOICE(2,"_deref_p");
  }
  switch(c2) {
    CHOICE(0,"d");
    CHOICE(1,"s");
    CHOICE(2,"c");
  }
  switch(c3) {
    CHOICE(0,"u");
    CHOICE(1,"s");
  }
  switch(c4) {
    CHOICE(0,"c");
    CHOICE(1,"z");
  }
  switch(c5) {
    CHOICE(0,"1");
    CHOICE(1,"2");
  }
  
  // Return an error if illegal value
  if (idx) return NULL;

  return lvalue_name_ret;
}

char rvalue_name_ret[1024];
char *rvalue_name(int idx)
{
  /* 
     rvalue=[_hi_|_lo_|]<lvalue>[[_rol_|_ror_][1-32|<lvalue>]]
  */
  int c1,c2,c3,c4=0;

  DEFCHOICE(1,3);
  DEFCHOICE(2,LVALUE_FORMS);
  DEFCHOICE(3,3);
  DEFCHOICE(4,(32+LVALUE_FORMS));
  if (!c3) c4=0;

  int ofs=0;
  switch(c1) {
    RCHOICE(0,"");
    RCHOICE(1,"_hi_");
    RCHOICE(2,"_lo_");
  }
  char *lvalue=lvalue_name(c2);
  snprintf(&rvalue_name_ret[ofs],1024-ofs,"%s",lvalue); ofs+=strlen(lvalue);
  switch(c3) {
    RCHOICE(0,"");
    RCHOICE(1,"_rol_");
    RCHOICE(2,"_ror_");
  }
  if (c3) {
    if (c4<32) {
      char num[10];
      snprintf(num,10,"{%d}",c4+1);
      snprintf(&rvalue_name_ret[ofs],1024-ofs,"%s",num); ofs+=strlen(num);
    } else {
      lvalue=lvalue_name(c3-32);
      snprintf(&rvalue_name_ret[ofs],1024-ofs,"%s",lvalue); ofs+=strlen(lvalue+2);
    }
  }
    
  // Return an error if illegal value
  if (idx) {
    return NULL;
  }

  return rvalue_name_ret;
  
}

int main(int argc,char **argv)
{
  for(int i=0;;i++) {
    if (!rvalue_name(i)) break;
    printf("%d: %s\n",i,rvalue_name(i));
  }
  
}
