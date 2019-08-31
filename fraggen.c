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

#define OP_AND 1
#define OP_OR 2
#define OP_XOR 3
#define OP_PLUS 4
#define OP_MINUS 5

struct thing {
  int reg_a;
  int reg_x;
  int reg_y;
  int bytes;
  int mem;
  int hi;
  int lo;
  int inc;
  int deref;
  int sign;
  int shift;
  int pointer;
  struct thing *shift_thing;
  int arg_op;
  struct thing *arg_thing;
  char *name;
  struct thing *derefidx;
};

struct thing *parse_thing(char *left);

void parse_thing_common(char *left,struct thing *t)
{
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

  //  fprintf(stderr,"Parsing '%s'\n",left);
  
  // d/w/b for size
  switch(left[1]) {
  case 'd': t->bytes=4; break;
  case 'w': t->bytes=2; break;
  case 'p': t->bytes=2; break;
  case 'b': t->bytes=1; break;
  default:
    fprintf(stderr,"Can't parse size '%c'\n",left[1]);
    exit(-1);
  }
  
  switch(left[2]) {
  case 't':
    // I think this means that we copy the size from the source?
    t->sign=0; t->bytes=99; break;
  case 's': t->sign=1; break;
  case 'u': t->sign=0; break;
  default:
    fprintf(stderr,"Can't parse signedness description '%c'\n",left[2]);
    exit(-1);
  }
  
  switch(left[3]) {
  case 'a':
    t->reg_a=1;
    break;
  case 'x':
    t->reg_x=1;
    break;
  case 'y':
    t->reg_y=1;
    break;
  case 'z':
    // There is an extra level of indirection for Z versus C.
    // Do we need to do anything else here to implement it?
    t->deref++;
  case 'c':
    t->name=&left[3];
    if (strchr(t->name,'_')) {
      char *suffix=strchr(t->name,'_');
      suffix[0]=0;
      suffix++;
      if (!strncmp(suffix,"ror_",4)) {
	if (sscanf(&suffix[4],"%d",&t->shift)==1) {
	  // Fixed numeric shift to the right
	} else {
	  t->shift=1; // RIGHT
	  t->shift_thing=parse_thing(&suffix[4]);
	}
	suffix=NULL;
      }
      else if (!strncmp(suffix,"rol_",4)) {
	if (sscanf(&suffix[4],"%d",&t->shift)==1) {
	  // Fixed numeric shift to the left
	  t->shift=-t->shift;    
	} else {
	  t->shift=-1; // LEFT
	  t->shift_thing=parse_thing(&suffix[4]);
	}
	suffix=NULL;	
      }
      else if (!strncmp(suffix,"band_",5)) {
	t->arg_op=OP_AND;
	t->arg_thing=parse_thing(&suffix[5]);
      }
      else if (!strncmp(suffix,"bor_",4)) {
	t->arg_op=OP_OR;
	t->arg_thing=parse_thing(&suffix[4]);
      }
      else if (!strncmp(suffix,"bxor_",5)) {
	t->arg_op=OP_XOR;
	t->arg_thing=parse_thing(&suffix[5]);
      }
      else if (!strncmp(suffix,"plus_",6)) {
	t->arg_op=OP_PLUS;
	t->arg_thing=parse_thing(&suffix[6]);
      }
      else if (!strncmp(suffix,"minus_",6)) {
	t->arg_op=OP_MINUS;
	t->arg_thing=parse_thing(&suffix[6]);
      }
      if (suffix&&suffix[0]) {
	if (!strncmp(suffix,"derefidx_",strlen("derefidx_"))) {
	  suffix+=strlen("derefidx_");
	  // Now parse the derefence index term.
	  // Skip any opening brackets
	  if (suffix[0]=='(') {
	    suffix++;
	    int len=strlen(suffix);
	    if (suffix[len-1]!=')') {
	      fprintf(stderr,"Unbalanced brackets\n");
	      exit(-1);
	    }
	    suffix[len-1]=0;
	  }
	  //	  fprintf(stderr,"Trying to parse suffix '%s'\n",suffix);
	  t->derefidx=parse_thing(suffix);
	  t->deref++;
	  suffix=NULL;
	}
      }

      if (suffix) {
	// XXX - derference indexes will have to be supported here in time
	fprintf(stderr,"Can't parse destination description suffix '_%s'.\n",suffix);
	exit(-1);
      }
    }
    break;
  default:
    fprintf(stderr,"Can't parse target description '%s'\n",&left[3]);
    exit(-1);
  }

  left+=4;
  
  return;
}
    


struct thing *parse_thing(char *left)
{
  struct thing *t=calloc(sizeof(struct thing),1);

  //  fprintf(stderr,"Parsing '%s'\n",left);
  
  if(!strncmp(left,"_hi_",4)) {
    t->hi=1;
    left+=strlen("_hi_");
  }
  if(!strncmp(left,"_lo_",4)) {
    t->lo=1;
    left+=strlen("_lo_");
  }
  while(!strncmp(left,"_inc_",5)) {
    t->inc++;
    left+=strlen("_inc_");
  }
  while(!strncmp(left,"_dec_",5)) {
    t->inc--;
    left+=strlen("_dec_");
  }
  while(!strncmp(left,"_deref_",7)) {
    t->deref++;
    left+=strlen("_deref_");
    if (left[0]=='(') {
      left++;
      int len=strlen(left);
      if (left[len-1]==')') left[len-1]=0;
    }
  }
  
  if (!strcmp(left,"vbuaa")) {
    t->reg_a=1; t->bytes=1;
  }
  else if (!strcmp(left,"vbuxx")) {
    t->reg_x=1; t->bytes=1;
  }
  else if (!strcmp(left,"vbuyy")) {
    t->reg_y=1; t->bytes=1;
  }
  else if (!strcmp(left,"vbsaa")) {
    t->reg_a=1; t->bytes=1; t->sign=1;
  }
  else if (!strcmp(left,"vbsxx")) {
    t->reg_x=1; t->bytes=1; t->sign=1;
  }
  else if (!strcmp(left,"vbsyy")) {
    t->reg_y=1; t->bytes=1; t->sign=1;
  }
  else if (left[0]=='p') {
    // It's a pointer, so 16 bits
    t->mem=1; t->bytes=2;

    t->pointer=1;

    parse_thing_common(left,t);       
  }
  else if (left[0]=='v') {

    t->mem=1;
    t->pointer=0;
    parse_thing_common(left,t);   
    
  }
  else {
    fprintf(stderr,"Don't know how to parse left argument '%s'\n",left);
    exit(-1);
  }

  return t;
}   

void describe_thing(int depth,struct thing *t)
{
  for(int i=0;i<depth;i++) printf(" ");
  if (t->reg_a) printf("reg_a");
  if (t->reg_x) printf("reg_x");
  if (t->reg_y) printf("reg_y");
  printf(" bytes=%d",t->bytes);
  if (t->hi) printf(" <"); 
  if (t->lo) printf(" >");
  if (t->inc) printf(" %-d",t->inc);
  if (t->deref) {
    printf(" ");
    for(int i=0;i<t->deref;i++) printf("*");
  }
  if (t->sign) printf(" signed");
  if (t->pointer) printf(" pointer");
  if (t->mem) printf(" mem");
  if (t->name) {
    printf(" name=%s",t->name);
  }
  if (t->derefidx) {
    printf("\n  deref:\n");
    describe_thing(depth+6,t->derefidx);
  }
  if (t->shift_thing) {
    printf("shift:\n");
    describe_thing(depth+6,t->shift_thing);
  }
  printf("\n");
}

int generate_assignment(char *left, char *right)
{
  struct thing *l,*r;

  l=parse_thing(left);
  r=parse_thing(right);

#if 0
  printf("Left:\n");
  describe_thing(0,l);
  printf("Right:\n");
  describe_thing(0,r);
#endif
  
  /*
    Ok, so now we have the parsed left and right expressions.
    We need to figure out how to get the things done.

    There are a bunch of complicated cases, but in general it boils down
    to copying bytes from the destination to the source, optionally
    adding, subtracting or shifting along the way.

    For the more complex cases, we first transform them into simpler
    cases by evaluating the complicated things, so that we end up
    with, at worst, a pointer or variable receiving the value of a pointer
    or variable.    

    If both sides need derefencing, then they have to be normalised to
    having the same offset from the pointer, so that Y can be used as the
    index for both.
    
  */

  if (l->bytes==99) l->bytes=r->bytes;
  if (r->bytes==99) {
    printf("ERROR: 't' tag to take source type cannot appear in rvalue\n");
    exit(-1);
  }
  
  // Do any setup we need, e.g., for pointer access
  if (l->deref>1||r->deref>1) {
    printf("ldy #0\n");
    if (l->deref==3) {
      // Read the pointer value
      // (avoiding the A register if that is the source value)
      if (r->reg_a) {
	printf("ldy {%s}\n",l->name);
	printf("sty !+ +1\n");
      } else {
	printf("lda {%s}\n",l->name);
	printf("sta !+ +1\n");
      }
    }
  }

  if (r->inc==1&&!strcmp(r->name,l->name)
      &&r->deref==1&&l->deref==1&&r->bytes==1&&l->bytes) {
    // Short cut for INC
    printf("inc {%s}\n",r->name);
  } else if (r->inc==-1&&!strcmp(r->name,l->name)
      &&r->deref==1&&l->deref==1&&r->bytes==1&&l->bytes) {
    // Short cut for DEC
    printf("dec {%s}\n",r->name);
  }
  else {

    // Normalise shifts down to a single bytes worth
    int shift_offset=r->shift/8;   
    r->shift-=shift_offset*8;
    l->shift-=shift_offset*8;
    
    //    printf("shift_offset=%d, r->shift=%d\n",shift_offset,r->shift);
    
    for(int byte=0;byte<4;byte++)
      {
	//      printf("; byte %d, deref = %d,%d\n",byte,l->deref,r->deref);
	
	// Stop once we have no more bytes to deal with
	if (l->bytes<=byte&&r->bytes<=byte) break;
	
	// only increment Y if there are more bytes coming,
	// and we have sufficient dereferencing to make it needed
	// i.e., on all but the first round
	if (byte)
	  if (l->deref>1||r->deref>1)
	    printf("iny\n");
	
	if (!byte) {
	  if (shift_offset<0) printf("lda #0\n");
	}
	
	if (byte<r->bytes) {
	  if (r->reg_a) {
	    // Nothing to do
	  } else if (r->reg_x) {
	    printf("txa\n");
	  } else if (r->reg_y) {
	    printf("tya\n");
	  } else if (r->deref==0) {
	    if (shift_offset+byte>=0) {
	      switch(byte) {
	      case 0: printf("lda #<{%s}\n",r->name); break;
	      case 1: printf("lda #>{%s}\n",r->name); break;
	      case 2: printf("lda #<{%s}>>16\n",r->name); break;
	      case 3: printf("lda #>{%s}>>16\n",r->name); break;
	      }
	    }
	  } else if (r->deref==1) {
	    if (shift_offset+byte>=0) {
	      printf("lda {%s}",r->name);
	      if (byte+shift_offset) printf("+%d",byte+shift_offset);
	      if (r->derefidx&&r->derefidx->reg_x) printf(",x");
	      else if (r->derefidx&&r->derefidx->reg_x) printf(",y");
	      else if (r->derefidx)
		printf("[UNIMPLMENENTED DEREF]\n");
	      printf("\n");
	    }
	  } else if (r->deref==2) {
	    if (shift_offset+byte>=0) {
	      printf("lda ({%s}),y\n",r->name);
	    }
	  } else if (r->deref==3) {
	    if (shift_offset+byte>=0) {
	      printf("UNIMPLEMENTED\n");
	    }
	  } else if (r->deref>3) {
	    fprintf(stderr,"ERROR: At most only two levels of dereference may be used.\n");
	    exit(-1);
	  } else {
	    fprintf(stderr,"ERROR: Don't know how to read from this source.\n");
	    exit(-1);
	  }
	} else {
	  if (byte==r->bytes) printf("lda #$00\n");
	}
	
	// Implement addition/subtraction of constants other than 1 and -1
	if (r->inc) {
	  if (!byte) printf("clc\n");
	  switch(byte) {
	  case 0: printf("adc #<{%s}\n",r->name); break;
	  case 1: printf("adc #>{%s}\n",r->name); break;
	  case 2: printf("adc #<{%s}>>16\n",r->name); break;
	  case 3: printf("adc #>{%s}>>16\n",r->name); break;
	  }	 
	}
	if (r->shift&&!r->shift_thing) {
	}
	if (r->shift&&r->shift_thing) {
	  printf("ERROR: Shifting by non-constant amounts not implemented.\n");
	}
	
	
	if (byte<l->bytes) {
	  if (l->reg_a) {
	    // Nothing to do
	  } else if (l->reg_x) {
	    printf("tax\n");
	  } else if (l->reg_y) {
	    printf("tay\n");       
	  } else if (l->deref==0) {
	    printf("ERROR: Writing to variables with no de-reference doesn't make sense.\n");	
	  } else if (l->deref==1) {
	    printf("sta {%s}",l->name);
	    if (byte) printf("+%d",byte);
	    printf("\n");
	  } else if (l->deref==2) {
	    printf("sta ({%s}),y\n",l->name);
	  } else if (l->deref==3) {
	    // For triple derefence, we will have setup the target pointer
	    // via self-modifying code
	    printf("!: sta ($ff),y\n");
	  } else if (l->deref>3) {
	    fprintf(stderr,"ERROR: At most only three levels of dereference may be used.\n");
	    exit(-1);
	  } else {
	    fprintf(stderr,"ERROR: Don't know how to write to this target.\n");
	    exit(-1);
	  }
	}
	
      }

    if (r->shift) {
      // Apply any remaining shifts to the target at the end
      
      for(int i=0;i<-r->shift;i++) {
	
	switch (l->deref) {
	case 1:
	  for(int b=1;b<=l->bytes;b++) {
	    if (b==1)
	      printf("asl {%s}",l->name);
	    else
	      printf("rol {%s}",l->name);
	    if (b>1) printf("+%d",b-1);
	    printf("\n");	    
	  }
	  break;
	case 2:
	  printf("ldy #%d\n",0);
	  for(int b=1;b<=l->bytes;b++) {
	    if (b==1)
	      printf("asl ({%s}),y",l->name);
	    else
	      printf("rol ({%s}),y",l->name);
	    if (b>1) printf("iny\n");
	    printf("\n");	    
	  }
	  break;
	default:
	  printf("ERROR: Shifting of non-dereferenced/excessively-deferencened targets not supported.\n");
	  break;
      }


      for(int i=0;i<r->shift;i++) {

	switch (l->deref) {
	case 1:
	  for(int b=l->bytes;b;b--) {
	    if (b==l->bytes)
	      printf("lsr {%s}",l->name);
	    else
	      printf("ror {%s}",l->name);
	    if (b>1) printf("+%d",b-1);
	    printf("\n");	    
	  }
	  break;
	case 2:
	  printf("ldy #%d\n",l->bytes-1);
	  for(int b=l->bytes;b;b--) {
	    if (b==l->bytes)
	      printf("lsr ({%s}),y",l->name);
	    else
	      printf("ror ({%s}),y",l->name);
	    if (b>1) printf("dey\n");
	    printf("\n");	    
	  }
	  break;
	default:
	  printf("ERROR: Shifting of non-dereferenced/excessively-deferencened targets not supported.\n");
	  break;
	}
	
      }
      }  
    }
  } 
  return 0;
}

int generate_comparison(char *destination,char *comparison)
{
  fprintf(stderr,"comparison handling not supported.\n");
  exit(-1);
  return 0;
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
  //  printf("Checking LVALUE_FORMS and RVALUE_FORMS...\n");
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
