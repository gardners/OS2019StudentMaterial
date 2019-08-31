#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

/*

  lvalue=[v|p|_deref_p][d|s|c][u|s][c|z][1|2]
  rvalue=[_hi_|_lo_|]<lvalue>[[_rol_|_ror_][1-32|<lvalue>]]
  
  <lvalue>=<rvalue>
*/

#define CHOICE(N,M)   case N: snprintf(&lvalue_name_ret[ofs],1024-ofs,"%s",M"p"); ofs+=strlen(M); break
#define RCHOICE(N,M)   case N: snprintf(&rvalue_name_ret[ofs],1024-ofs,"%s",M"p"); ofs+=strlen(M); break
#define DEFCHOICE(N,WAYS) c##N=idx%WAYS; idx=idx/WAYS

// XXX These need to be updated manually if the functions change
#define LVALUE_FORMS 72
#define RVALUE_FORMS 67392

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

struct thing {
  int reg_a;
  int reg_x;
  int reg_y;
  int bits;
  int mem;
  int hi;
  int lo;
  int deref;
  int sign;
  int shift;
  char *name;
  struct thing *derefidx;
};

struct thing *parse_thing(char *left)
{
  struct thing *t=calloc(sizeof(struct thing),1);
  
  if(!strncmp(left,"_hi_",4)) {
    t->hi=1;
    left+=strlen("_hi_");
  }
  if(!strncmp(left,"_lo_",4)) {
    t->lo=1;
    left+=strlen("_lo_");
  }
  while(!strncmp(left,"_deref_",8)) {
    t->deref++;
    left+=strlen("_deref_");
  }
  
  if (!strcmp(left,"vbuaa")) {
    t->reg_a=1; t->bits=8;
  }
  else if (!strcmp(left,"vbuxx")) {
    t->reg_x=1; t->bits=8;
  }
  else if (!strcmp(left,"vbuyy")) {
    t->reg_y=1; t->bits=8;
  }
  else if (!strcmp(left,"vbsaa")) {
    t->reg_a=1; t->bits=8; t->sign=1;
  }
  else if (!strcmp(left,"vbsxx")) {
    t->reg_x=1; t->bits=8; t->sign=1;
  }
  else if (!strcmp(left,"vbsyy")) {
    t->reg_y=1; t->bits=8; t->sign=1;
  }
  else if (left[0]=='p') {
    // It's a pointer, so 16 bits
    t->mem=1; t->bits=16;
  }
  else if (left[0]=='v') {
    // d/w/b for size
    switch(left[1]) {
    case 'd': t->bits=32; break;
    case 'w': t->bits=16; break;
    case 'b': t->bits=8; break;
    default:
      fprintf(stderr,"Can't parse size '%c'\n",left[1]);
      exit(-1);
    }

    switch(left[2]) {
    case 's': t->sign=1; break;
    case 'u': t->sign=0; break;
    default:
      fprintf(stderr,"Can't parse signedness description '%c'\n",left[2]);
      exit(-1);
    }

    switch(left[3]) {
    case 'z':
      t->name=&left[3];
      if (strchr(t->name,'_')) {
	// XXX - derference indexes will have to be supported here in time
	fprintf(stderr,"Can't parse destination description '%s', due to _ suffix.\n",&left[3]);
	exit(-1);
      }
      break;
    default:
      fprintf(stderr,"Can't parse signedness description '%s'\n",&left[3]);
      exit(-1);
    }
  }
  else {
    fprintf(stderr,"Don't know how to parse left argument '%s'\n",left);
    exit(-1);
  }

  return t;
}   
  
int generate_assignment(char *left, char *right)
{
  struct thing *l,*r;

  l=parse_thing(left);
  r=parse_thing(right);
  
  /*
    The right value is more complex, as we can have a pile of levels of 
    de-referencing to do. We'll add those later.

    From Jesper:
    J: Pointers are a bit tricky - especially since c1 and z1 adds some more complexity
    J: c1 means the value is constant. So pbuc1 is a pointer to a byte - where the pointer itself is a constant address I. Memory
    J: pbuz1 is a pointer to a byte where the pointer is stored on ZP
    So there is an extra level of indirection for z’s
    Me: So what does pptc1 mean exactly then?
    J: Pointer to pointer (constant address) - so far pointers to pointers at not typed beyond that.    

  */

  printf("lvalue is %d bits, target is %s\n",
	 l->bits,l->name?l->name:"probably a register");

  printf("rvalue is %d bits, source is %s\n",
	 r->bits,r->name?r->name:"probably a register");  
  
  return 0;
}

int generate_comparison(char *destination,char *comparison)
{
}

int generate_fragment(char *name)
{
  /*
    First check, is it an assignment or comparison+then ?
  */
  if (strchr(name,'=')) {
    // Assignment
    char left[1024];
    char right[1024];
    int offset;
    if (sscanf(name,"%[^=]=%[^=]%n",left,right,&offset)!=2) {
      fprintf(stderr,"Could not parse assignment fragment '%s' (couldn't separate left and right sides)\n",name);
      exit(-1);
    }
    if (offset!=strlen(name)) {
      fprintf(stderr,"Could not parse assignment fragment '%s' (junk at end of name)\n",name);
      exit(-1);
    }
    return generate_assignment(left,right);
  } else {
    // Comparison + branch
    char *then=strstr(name,"_then_");
    if (!then) {
      fprintf(stderr,"Could not parse fragment '%s' (neither assignment nor THEN fragment?)\n",name);
      exit(-1);
    }
    int then_ofs=then-name;
    char comparison[1024];
    char *destination=then+strlen("_then_");
    strcpy(comparison,name);
    comparison[then_ofs]=0;

    return generate_comparison(destination,comparison);

  }
  
  return 0;
}

int main(int argc,char **argv)
{
  int i;
  printf("Checking LVALUE_FORMS and RVALUE_FORMS...\n");
  for(i=0;;i++) {
    if (!lvalue_name(i)) break;
  }
  if (i!=LVALUE_FORMS) {
    fprintf(stderr,"LVALUE_FORMS should be %d, not %d. Please update source\n",i,LVALUE_FORMS);
    exit(-1);
  }
  for(i=0;;i++) {
    if (!rvalue_name(i)) break;
  }
  if (i!=RVALUE_FORMS) {
    fprintf(stderr,"RVALUE_FORMS should be %d, not %d. Please update source\n",i,RVALUE_FORMS);
    exit(-1);
  }

  if (argv[1]) {
    generate_fragment(argv[1]);
  }
  
  return 0;
}
