#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

/*

  lvalue=[v|p|_deref_p][d|s|c][u|s][c|z][1|2]
  rvalue=[_hi_|_lo_|]<lvalue>[[_rol_|_ror_][1-32|<lvalue>]]
  
  <lvalue>=<rvalue>
*/

// Comparison operations
#define GE 1
#define GT 2
#define LE 3
#define LT 4
#define EQ 5
#define NE 6

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
  int early_deref;
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

  //  fprintf(stderr,"parse_thing_common: Parsing '%s'\n",left);
  
  // d/w/b for size
  switch(left[1]) {
  case 'd': t->bytes=4; break;
  case 'w': t->bytes=2; break;
  case 'p': t->bytes=2; break;
  case 's': t->bytes=2; break;
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
  case 'r': t->sign=0; break; // pointer to a function
  default:
    fprintf(stderr,"Can't parse signedness description '%c'\n",left[2]);
    exit(-1);
  }
  
  switch(left[3]) {
  case 'a':
    t->reg_a=1;
    t->name=&left[3];
    break;
  case 'x':
    t->reg_x=1;
    t->name=&left[3];
    break;
  case 'y':
    t->reg_y=1;
    t->name=&left[3];
    break;
  case 'z':
    // There is an extra level of indirection for Z versus C.
    // Do we need to do anything else here to implement it?
    t->deref++;
  case 'c':
    t->name=&left[3];
    break;
  default:
    fprintf(stderr,"Can't parse target description '%s'\n",&left[3]);
    exit(-1);
  }

  if (!t->name) {
    printf("t->name=NULL from left='%s'\n",left);
  }
  
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
      suffix=NULL;	
    }
    else if (!strncmp(suffix,"bor_",4)) {
      t->arg_op=OP_OR;
      t->arg_thing=parse_thing(&suffix[4]);
      suffix=NULL;	
    }
    else if (!strncmp(suffix,"bxor_",5)) {
      t->arg_op=OP_XOR;
      t->arg_thing=parse_thing(&suffix[5]);
      suffix=NULL;	
    }
    else if (!strncmp(suffix,"plus_",5)) {
      t->arg_op=OP_PLUS;
      t->arg_thing=parse_thing(&suffix[5]);
      suffix=NULL;	
    }
    else if (!strncmp(suffix,"minus_",6)) {
      t->arg_op=OP_MINUS;
      t->arg_thing=parse_thing(&suffix[6]);
      suffix=NULL;	
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

  return;
}
    


struct thing *parse_thing(char *left)
{
  struct thing *t=calloc(sizeof(struct thing),1);

  //  fprintf(stderr,"parse_thing: Parsing '%s'\n",left);
  
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
  while(!strncmp(left,"_ptr_",5)) {
    // Cast to pointer
    left+=strlen("_ptr_");
  }
  while(!strncmp(left,"_word_",5)) {
    // Cast to pointer
    left+=strlen("_word_");
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
    if (left[1]=='p') t->pointer=2;

    parse_thing_common(left,t);       
  }
  else if (left[0]=='v') {

    t->mem=1;
    t->pointer=0;
    parse_thing_common(left,t);   
    
  }
  else if (!strncmp(left,"(_deref_",8)) {
    // This means we have something we have to derefence first
    // We can handle this by first resolving the thing in brackets,
    // and then re-writing the remainder to something that we can
    // process
    char name[1024];
    strcpy(name,&left[8]);
    char *close=strchr(&left[1],')');
    name[close-left-8]=0;
    //    printf("early deref. name='%s',remainder='%s'\n",name,&close[1]);
    char rewritten[1024];
    snprintf(rewritten,1024,"%s%s",name,&close[1]);
    //    printf("Parsing rewritten form '%s'\n",rewritten);
    struct thing *t2=parse_thing(rewritten);
    if (!t2) {
      printf("Failed to parse re-written fragment '%s'\n",rewritten);
    }
    free(t);
    t=t2;
    t->early_deref++;
    t->deref++;
  } else if (left[0]>='0'&&left[0]<='9') {
    // Literal number
    t->name=left;
    t->bytes=99;
  }  else {
    fprintf(stderr,"Don't know how to parse left argument '%s'\n",left);
    exit(-1);
  }

  return t;
}   

void dump_mem(char *msg,void *m,int size)
{
  unsigned char *p=m;

  printf("%s (%p):\n",msg,m);
  for(int i=0;i<size;i+=16) {
    printf("%08x : ",i);
    for(int j=0;j<16;j++) {
      if (i+j<size) printf("%02x ",p[i+j]); else printf("   ");
    }
    printf("  ");
    for(int j=0;j<16;j++) {
      if (i+j<size) {
	if (p[i+j]>=0x20&&p[i+j]<0x7e)
	  printf("%c",p[i+j]);
	else
	  printf(".");
      } else printf(" ");
    }
    printf("\n");
  }
  printf("\n");
}

void describe_thing(int depth,struct thing *t)
{
  dump_mem("t",t,sizeof(*t));
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
    printf("\n  derefidx %p:\n",t->derefidx);
    describe_thing(depth+6,t->derefidx);
  }
  if (t->shift_thing) {
    printf("shift:\n");
    describe_thing(depth+6,t->shift_thing);
  }
  if (t->arg_thing) {
    printf("\n  arith op:\n");
    describe_thing(depth+6,t->arg_thing);
  }
  printf("\n");
}

void expand_op(int byte,struct thing *r)
{
  if (r->arg_thing) {
    char name[1024]="{ERROR: Could not resolve}";
    if (r->arg_thing->name)
      snprintf(name,1024,"{%s}",r->arg_thing->name);
    if (r->arg_thing->reg_a) {
      printf("ERROR: Reading arg from A register. Does this ever make sense?\n");
    }
    if (r->arg_thing->reg_x) {
      //      printf("ERROR: Reading arg from X register.\n");
      printf("stx $f0\n");
      snprintf(name,1024,"$f0");
    }
    if (r->arg_thing->reg_y) {
      //      printf("ERROR: Reading arg from Y register.\n");
      printf("sty $f0\n");
      snprintf(name,1024,"$f0");
    }
    //    if (r->arg_thing->bytes>1) {
    //  printf("ERROR: We don't (yet) support >1 bytes here.\n");
    //}
    switch(r->arg_op) {
    case OP_AND:
      printf("and %s\n",name);
      break;
    case OP_OR:
      printf("ora %s\n",name);
      break;
    case OP_XOR:
      printf("eor %s\n",name);
      break;
    case OP_MINUS:
      if (!byte) printf("sec\n");
      switch (r->arg_thing->deref) {
      case 0:
	switch(byte) {
	case 0: printf("sbc #<%s\n",name); break;
	case 1: printf("sbc #>%s\n",name); break;
	case 2: printf("sbc #<%s>>16\n",name); break;
	case 3: printf("sbc #>%s>>16\n",name); break;
	}
	break;
      case 1:
	switch(byte) {
	case 0: printf("sbc %s\n",name); break;
	case 1: printf("sbc %s+1\n",name); break;
	case 2: printf("sbc %s+2\n",name); break;
	case 3: printf("sbc %s+3\n",name); break;
	}
	break;
      default:
	printf("ERROR: not implemented.\n");
      }      
      break;
    case OP_PLUS:
      if (!byte) printf("clc\n");
      switch (r->arg_thing->deref) {
      case 0:
	switch(byte) {
	case 0: printf("adc #<%s\n",name); break;
	case 1: printf("adc #>%s\n",name); break;
	case 2: printf("adc #<%s>>16\n",name); break;
	case 3: printf("adc #>%s>>16\n",name); break;
	}
	break;
      case 1:
	switch(byte) {
	case 0: printf("adc %s\n",name); break;
	case 1: printf("adc %s+1\n",name); break;
	case 2: printf("adc %s+2\n",name); break;
	case 3: printf("adc %s+3\n",name); break;
	}
	break;
      case 2:
	switch(byte) {
	case 0: printf("adc (%s),y\n",name); break;
	case 1: printf("adc (%s),y\n",name); break;
	case 2: printf("adc (%s),y\n",name); break;
	case 3: printf("adc (%s),y\n",name); break;
	}
	break;
      default:
	printf("ERROR: not implemented.\n");
      }      
      break;
      break;
    }
  }
  
}

int deref2_uses_y(struct thing *r)
{
  if (r->derefidx&&r->derefidx->reg_y) return 1;
  if (r->arg_thing&&r->arg_thing->reg_y) return 1;
  if (r->arg_thing&&r->arg_thing->derefidx&&r->arg_thing->derefidx->reg_y) return 1;
  return 0;
 
}

void expand_arith_interim_step(int comparison_op,int byte,struct thing *l,char *branch_target, int reverse_order)
{
  if (0) printf("op=%d,byte=%d, rev=%d, l->bytes=%d, l->sign=%d\n",
		comparison_op,byte,reverse_order,l->bytes,l->sign);  

  switch(comparison_op) {
  case EQ:
    
    if (byte<(l->bytes-1)) printf("bne !+\n");
    break;
    
  case NE:
    printf("bne {%s}\n",branch_target); break;
    
  case GE:
    if (!l->sign) {
      if (byte||(!reverse_order))
	printf("bcc !+\nbne {%s}\n",branch_target);
    }
    break;
  case GT: 
    if (byte) {
      printf("bcc {%s}\nbne !+\n",branch_target);
    }
    break;
  case LT:
    if (byte) {
      printf("bcc {%s}\nbne !+\n",branch_target);
    }
    break;
  case LE:    
    if (byte) {
      //   if (!reverse_order)
	printf("bne !+\n");
      //      else
      //	printf("bcc {%s}\nbne !+\n",branch_target);	
    }
    break;
  }		
}
  
int generate_assignment(char *left, char *right,int comparison_op,char *branch_target)
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

  if (r->bytes==99) {
    // If the rvalue is 't', then it is just a normal pointer
    r->bytes=2;
  }
  if (l->bytes==99) {
    //    printf("copying l->bytes from r->bytes\n");
    l->bytes=r->bytes;
  }

  //  printf("%d %d %d %d\n",l->pointer,l->deref,r->pointer,r->deref);
  if ((r->pointer>r->deref)||
      (l->pointer>l->deref)||
      (!strncmp("_deref_pp",left,9))) {
    // Pointers
    //    printf("Inferring pointers %d.\n",__LINE__);
    r->bytes=2;
    l->bytes=2;
  }
  // Yet another pointer inference kludge
  if (((left[0]=='p')||!strncmp(left,"_ptr_",5))
      &&
      ((right[0]=='p')||!strncmp(right,"_ptr_",5))
      &&(!l->derefidx)
      &&(!r->derefidx)
      ) {
    // printf("Inferring pointers %d.\n",__LINE__);
    r->bytes=2;
    l->bytes=2;
  }
  
  //  describe_thing(0,l);

  int simple_pointer_cast=0;
  int valid_bytes=1;
  int skip_dey=1;
  int byte=0;
  int reverse_order=0;
  if ((!l->sign)&&comparison_op) {
    //    printf("unsigned comparison -- consider reversing order\n");
    switch(comparison_op) {
    case GE: case GT: case LE: case LT:
      reverse_order=1;
      break;
    }
  }

  if (l&&r&&l->name&&r->name)
    if (!strcmp(l->name,r->name)) {
      if (l->deref==r->deref)
	if (!l->arg_thing)
	  if (!r->arg_thing)
	    if (!l->derefidx)
	      if (!r->derefidx)
		if (!l->inc)
		  if (!r->inc)
		    if (!l->shift_thing)		  
		      if (!r->shift_thing) {
			printf("// No operation needed\n");
			simple_pointer_cast=1;
		      }
      
    }

  if (l->bytes>valid_bytes) valid_bytes=l->bytes;
  if (r->bytes>valid_bytes) valid_bytes=r->bytes;  
  
  // Do any setup we need, e.g., for pointer access
  if (l->deref>1||r->deref>1) {
    //    printf("deref=%d\n",l->deref);
    switch(l->deref) {
    case 0:
      // No deref, nothing to do
      if (r->deref) printf("ldy #%d\n",reverse_order?(valid_bytes-1):0);
      break;
    case 1:
      printf("ldy #%d\n",reverse_order?(valid_bytes-1):0);
      break;
    case 2:
      if (l->derefidx&&!l->early_deref) {
	// We need to first add the deref index to
	// the address
	if (r->reg_a) {
	  // Avoid using A, since we want to use it
	  if (!l->derefidx->reg_y)
	    printf("ldy {%s}\n",l->derefidx->name);
	  printf("ldx {%s},y\n",l->name);
	  // Use self-modifying code to re-write pointer directly into
	  // target instruction
	  printf("stx !+ +1\n");
	  printf("ldx {%s}+1,y\n",l->name);
	  printf("stx !+ +2\n");
	  
	} else {
	  
	}
      } else {
	// De-ref pointer without offset
	// This means we can just use lda ($nn),y
	// But save Y first if both sides need to use this addressing mode,
	// and with different offsets
	if ((!l->derefidx||!l->derefidx->reg_y)) {
	  if (deref2_uses_y(r)) printf("sty $ff\n");
	  else {
	    printf("ldy #%d\n",reverse_order?(valid_bytes-1):0);
	  }
	}
	if (l->early_deref) {
	  if (r->reg_a) {
	    printf("ldx {%s}\n",l->name);
	    printf("stx $fe\n");
	    printf("ldx {%s}+1\n",l->name);
	    printf("stx $ff\n");
	  } else {
	    printf("lda {%s}\n",l->name);
	    printf("sta $fe\n");
	    printf("lda {%s}+1\n",l->name);
	    printf("sta $ff\n");
	  }
	}
      }
      break;
    case 3:
      if (l->derefidx) {
	if (r->reg_a) printf("pha\n");
	printf("ldy #{%s}\n",l->derefidx->name);
	printf("lda ({%s}),y\n",l->name);
	printf("sta !+ +1\n");
	printf("iny\n");
	printf("lda ({%s}),y\n",l->name);
	printf("sta !+ +2\n");
	if (r->reg_a) printf("pla\n");
      } else {
	if (r->reg_a) {	
	  printf("ldy {%s}\n",l->name);
	  printf("sty !+ +1\n");
	} else {
	  printf("lda {%s}\n",l->name);
	  printf("sta !+ +1\n");
	}
	printf("ldy #0\n");
      }
      break;
    default:
      printf("ERROR: Too many dereferences\n");
    }
  } else if (l->deref==3) {
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

  if (r->inc==1&&!strcmp(r->name,l->name)
      &&r->deref==1&&l->deref==1&&r->bytes==1&&l->bytes) {
    
    // Short cut for INC
    switch(l->deref) {
    case 1:
      if (!l->derefidx)
	printf("inc {%s}\n",r->name);
      else {
	if (l->derefidx) {
	  if (l->derefidx->derefidx) {
	    printf("lda ({%s}),y\ntay\n",l->derefidx->derefidx->name);	    
	  } else {
	    printf("ldy #0\n");
	  }
	  printf("lda ({%s}),y\ntax\n",l->derefidx->name);
	}
	printf("inc {%s},x\n",r->name);
      }
      break;
    default:
      printf("ERROR: unsupported indirection for inferred INC instruction\n");
      break;
    }
  } else if (r->inc==1&&!strcmp(r->name,l->name)
      &&r->deref==1&&l->deref==1&&r->bytes==2&&l->bytes) {
    // Short cut for INC
    printf("inc {%s}\n",r->name);
    printf("bne !+\n");
    printf("inc {%s}+1\n",r->name);
    printf("!:\n");
  } else if (r->inc==-1&&!strcmp(r->name,l->name)
      &&r->deref==1&&l->deref==1&&r->bytes==1&&l->bytes) {
    // Short cut for DEC
    if (!l->derefidx)
      printf("dec {%s}\n",r->name);
    else {
      if (l->derefidx) {
	if (l->derefidx->derefidx) {
	  printf("lda ({%s}),y\ntay\n",l->derefidx->derefidx->name);	    
	} else {
	  printf("ldy #0\n");
	}
	printf("lda ({%s}),y\ntax\n",l->derefidx->name);
      }
      printf("dec {%s},x\n",r->name);
    }
  } else if (r->inc==-1&&!strcmp(r->name,l->name)
      &&r->deref==1&&l->deref==1&&r->bytes==2&&l->bytes) {
    // Short cut for DEC
    printf("dec {%s}\n",r->name);
    printf("bne !+\n");
    printf("dec {%s}+1\n",r->name);
    printf("!:\n");
  }
  else {

    // Normalise shifts down to a single bytes worth
    int shift_offset=r->shift/8;   
    r->shift-=shift_offset*8;
    l->shift-=shift_offset*8;
    
    //    printf("shift_offset=%d, r->shift=%d\n",shift_offset,r->shift);

    if (r->shift&&r->shift_thing) {
      // Handled further down
    }
    else {
      for(int rbyte=0;rbyte<4;rbyte++)
	{
	  if (reverse_order) byte=3-rbyte; else byte=rbyte;
	  
	  if (0)
	    printf("; byte %d, deref = %d,%d.  lbytes=%d, rbytes=%d\n",
		   byte,l->deref,r->deref,l->bytes,r->bytes);
	  
	  // Stop once we have no more bytes to deal with
	  if (l->bytes<=byte&&r->bytes<=byte) continue;
	  
	  // only increment Y if there are more bytes coming,
	  // and we have sufficient dereferencing to make it needed
	  // i.e., on all but the first round
	  if (byte)
	    if (l->deref>1||r->deref>1) {
	      // Get $00 into A for upper bytes by copying
	      // the zero value currently in Y
	      if (r->reg_a&&l->bytes>1) printf("tya\n");

	      if ((!l->derefidx||!l->derefidx->reg_y)) {
		if (deref2_uses_y(r)) printf("ldy $ff\n");
	      }
	      if (!reverse_order) printf("iny\n");
	    }
	  if (rbyte) 
	    if (l->deref>1||r->deref>1) {
	      if (reverse_order&&(!skip_dey))
		printf("dey\n");
	      skip_dey=0;
	    }
	  if (!byte) {
	    if (shift_offset<0) {
	      // No need for lda #0 when tya provides the value
	      if (!(r->reg_a&&l->bytes>1)) 
		printf("lda #0\n");
	    }
	  }
	  
	  if (byte<r->bytes) {
	    if (r->reg_a) {
	      // Nothing to do
	    } else if (r->reg_x) {
	      if (l->reg_a)
		printf("txa\n");
	    } else if (r->reg_y) {
	      if (l->reg_a)
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
		if (!simple_pointer_cast) {
		  printf("lda {%s}",r->name);
		  if (byte+shift_offset) printf("+%d",byte+shift_offset);
		  if (r->derefidx&&r->derefidx->reg_x) printf(",x");
		  else if (r->derefidx&&r->derefidx->reg_y) printf(",y");
		  else if (r->derefidx) {
		    printf("[UNIMPLMENENTED DEREF]\n");
		  }
		  printf("\n");
		}
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
	    if (byte==r->bytes) {
	      // No need for lda #0 when tya provides the value
	      if (!(r->reg_a&&l->bytes>1)) 
		printf("lda #0\n");
	    }
	  }
	  
	  // Implement addition/subtraction of constants other than 1 and -1
	  if (r->inc) {
	    int add=1;
	    if (r->inc<0) add=0;
	    if (r->bytes==1&&r->inc&0x80) add=0;
	    if (r->bytes==2&&r->inc&0x8000) add=0;
            if (add) {
	      if (!byte) printf("clc\n");
	      switch(byte) {
	      case 0: printf("adc #<$%x\n",r->inc); break;
	      case 1: printf("adc #>$%x\n",r->inc); break;
	      case 2: printf("adc #<$%x>>16\n",r->inc); break;
	      case 3: printf("adc #>$%x>>16\n",r->inc); break;
	      }
	    } else {
	      if (!byte) printf("sec\n");
	      if (r->inc==-1) {
		switch(byte) {
		case 0: printf("sbc #1\n"); break;
		default: printf("sbc #0\n"); break;
		}
	      } else {
		switch(byte) {
		case 0: printf("sbc #<%d\n",-r->inc); break;
		case 1: printf("sbc #>%d\n",-r->inc); break;
		case 2: printf("sbc #<%d>>16\n",-r->inc); break;
		case 3: printf("sbc #>%d>>16\n",-r->inc); break;
		}
	      }
	    }
	  }

	  if (byte<l->bytes) {
	    if (l->reg_a) {
	      // Nothing to do
	    } else if (l->reg_x) {
	      if (r->reg_a)
		printf("tax\n");
	    } else if (l->reg_y) {
	      if (r->reg_a)
		printf("tay\n");       
	    } else if (l->deref==0) {
	      if (!comparison_op) {
		printf("ERROR: Writing to variables with no de-reference doesn't make sense.\n");
	      } else {
		char *opcode="cmp";

		if (l->sign) {

		  // For signed comparisons we have to do the complete sequence and finish
		  // with SBC so that V flag gets set to indicate overflow or not.
		  if (byte>=(l->bytes-1)) {
		    switch(comparison_op) {
		    case GE: case LT: case LE: case GT: opcode="sbc";
		      break;
		    }
		  }
		} else {
		  // For unsigned comparisons we can either (1) use reversed byte order to
		  // short-circuit the comparison to save some cycles when the higher-order
		  // bytes reveal the result early, or (2) do the comparison in normal order
		  // and check if the result is negative, zero or positive.
		  // Jesper uses (1) because it is faster in some cases.
		  // This makes our life harder, because we have to enable reverse order
		}

		// No need to do CMP if comparing with zero
		if (strcmp(l->name,"0")) {
		  switch(byte) {
		  case 0: printf("%s #<{%s}\n",opcode,l->name); break;
		  case 1: printf("%s #>{%s}\n",opcode,l->name); break;
		  case 2: printf("%s #<{%s}>>16\n",opcode,l->name); break;
		  case 3: printf("%s #>{%s}>>16\n",opcode,l->name); break;
		  }
		}
		expand_arith_interim_step(comparison_op,byte,l,branch_target, reverse_order);
	      }
	    } else if (l->deref==1) {
	      if (!comparison_op) {
		if (r->reg_x) printf("stx {%s}",l->name);
		else if (r->reg_y) printf("sty {%s}",l->name);
		else {
		  expand_op(byte,r);
		  if (!simple_pointer_cast) {
		    if (!l->derefidx)
		      printf("sta {%s}",l->name);
		    else {
		      // Now resolve the derefencing thing
		      if (l->derefidx->derefidx) {
			if (l->derefidx->derefidx->reg_x) {
			  printf("ldy {%s},x\n",l->derefidx->name);
			  printf("sta {%s},y",l->name);
			} else if (l->derefidx->derefidx->reg_y) {
			  printf("ldx {%s},y\n",l->derefidx->name);
			  printf("sta {%s},x",l->name);
			} else {
			  printf("ERROR: Unsupported nested derefence form\n");
			}
		      }
		    }
		    if (byte) printf("+%d",byte);
		    printf("\n");
		  }
		}
	      } else {
		char *opcode="cmp";

		if (l->sign) {

		  // For signed comparisons we have to do the complete sequence and finish
		  // with SBC so that V flag gets set to indicate overflow or not.
		  if (byte>=(l->bytes-1)) {
		    switch(comparison_op) {
		    case GE: case LT: case LE: case GT: opcode="sbc";
		      break;
		    }
		  }
		} else {
		  // For unsigned comparisons we can either (1) use reversed byte order to
		  // short-circuit the comparison to save some cycles when the higher-order
		  // bytes reveal the result early, or (2) do the comparison in normal order
		  // and check if the result is negative, zero or positive.
		  // Jesper uses (1) because it is faster in some cases.
		  // This makes our life harder, because we have to enable reverse order
		}
		
		switch(byte) {
		case 0: printf("%s {%s}\n",opcode,l->name); break;
		case 1: printf("%s {%s}+1\n",opcode,l->name); break;
		case 2: printf("%s {%s}+2\n",opcode,l->name); break;
		case 3: printf("%s {%s}+3\n",opcode,l->name); break;
		}
		expand_arith_interim_step(comparison_op,byte,l,branch_target, reverse_order);
		
	      }
	    } else if (l->deref==2) {
	      // Reset Y if required
	      if ((!l->derefidx||!l->derefidx->reg_y)) {
		if (deref2_uses_y(r)) printf("ldy #%d\n",byte);
	      }

	      expand_op(byte,r);
	      
	      if (!comparison_op) {
		
		if (!l->derefidx||l->early_deref)  {
		  if (l->early_deref) 
		    printf("sta ($fe),y\n");
		  else
		    printf("sta ({%s}),y\n",l->name);
		}
		else
		  printf("!: sta $ffff\n");
	      } else {
		char *opcode="cmp";

		if (l->sign) {

		  // For signed comparisons we have to do the complete sequence and finish
		  // with SBC so that V flag gets set to indicate overflow or not.
		  if (byte>=(l->bytes-1)) {
		    switch(comparison_op) {
		    case GE: case LT: case LE: case GT: opcode="sbc";
		      break;
		    }
		  }
		} else {
		  // For unsigned comparisons we can either (1) use reversed byte order to
		  // short-circuit the comparison to save some cycles when the higher-order
		  // bytes reveal the result early, or (2) do the comparison in normal order
		  // and check if the result is negative, zero or positive.
		  // Jesper uses (1) because it is faster in some cases.
		  // This makes our life harder, because we have to enable reverse order
		}
		
		printf("%s ({%s}),y\n",opcode,l->name);

		expand_arith_interim_step(comparison_op,byte,l,branch_target, reverse_order);
	      }
		       
	    } else if (l->deref==3) {
	      // For triple derefence, we will have setup the target pointer
	      // via self-modifying code
	      if (r->reg_x) printf("txa\n");
	      if (r->reg_y) printf("tya\n");
	      if (l->derefidx)
		printf("!: sta $ffff\n");
	      else
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
    }
    
    if (r->shift) {
      // Apply any remaining shifts to the target at the end

      if (r->shift_thing) {
	// Non-constant shift
	/*	printf("ERROR: Non-constant shift not supported.\n");
		printf("   shift=%d, shift thing = ",r->shift);
		describe_thing(0,r->shift_thing); */

	// Get absolute count of shift
	int count=r->shift;
	if (count<0) count=-count;

	if (r->shift_thing->bytes!=1) {
	  printf("ERROR: shifting by constant must be 8-bit constant.\n");
	}

	// If left and right sides are different, then copy initial value of
	// right side to left
	if (strcmp(l->name,r->name)|(l->deref!=r->deref)) {
	  for(int b=1;b<=l->bytes;b++) {
	    switch (r->deref) {
	    case 1:
	      printf("lda {%s}",r->name);
	      if (b>1) printf("+%d",b-1);
	      printf("\n");
	      break;
	    case 2:
	      printf("lda ({%s}),y\n",r->name);
	      break;
	    default:
	      printf("ERROR: Max supported derefence level = 2\n");
	      break;
	    }
	    switch (l->deref) {
	    case 1:
	      printf("sta {%s}",r->name);
	      if (b>1) printf("+%d",b-1);
	      printf("\n");
	      break;
	    case 2:
	      printf("sta ({%s}),y\n",r->name);
	      break;
	    default:
	      printf("ERROR: Max supported derefence level = 2\n");
	      break;
	    }
	    if (l->deref>1||r->deref>1) {
	      // Get $00 into A for upper bytes by copying
	      // the zero value currently in Y
	      if (r->reg_a&&l->bytes>1) printf("tya\n");
	      printf("iny\n");
	    }
	  }
	  if (l->deref>1||r->deref>1) printf("ldy #0\n");
	}
	
	// Should thiss be #{c1} or {c1}?
	if (r->shift_thing->name[0]=='c')
	  printf("ldx #{%s}\n",r->shift_thing->name);
	else if (r->shift_thing->name[0]=='z') {
	  switch(r->shift_thing->deref) {
	  case 0:
	    printf("ldx #{%s}\n",r->shift_thing->name);
	    break;
	  case 1:
	    printf("ldx {%s}\n",r->shift_thing->name);
	    break;
	  case 2:
	    printf("lda ({%s}),y\ntax\n",r->shift_thing->name);
	    break;
	  default:
	    printf("ERROR: shift value must not be dereferenced > 2 levels.\n");
	  }
	} else {
	  printf("ERROR: shift value must be a constant or variable. I saw '%s'\n",r->shift_thing->name);
	}
	printf("!:\n");
	if (r->shift<0) {
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
	    printf("ldy #0\n");
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

	} else {
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
	printf("dex\n");
	printf("bne !-\n");
	  
      } else {

	// Handle constant shift left
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
	    printf("ldy #0\n");
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
	}

	// Handle constant shift right
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
  // We need to indicate if it is signed, so that patching the end of comparisons can work
  return l->sign;
}

int generate_comparison(char *destination,char *comparison)
{ 
  //  printf("Comparison = '%s'\n",comparison);

  int op;
  
  char *relation=NULL;
  char *right=NULL;
  if (!relation) { relation=strstr(comparison,"_ge_"); if (relation) op=GE; }
  if (!relation) { relation=strstr(comparison,"_gt_"); if (relation) op=GT; }
  if (!relation) { relation=strstr(comparison,"_le_"); if (relation) op=LE; }
  if (!relation) { relation=strstr(comparison,"_lt_"); if (relation) op=LT; }
  if (!relation) { relation=strstr(comparison,"_eq_"); if (relation) op=EQ; }
  if (!relation) { relation=strstr(comparison,"_neq_"); if (relation) op=NE; }

  if (!relation) {
    printf("ERROR: unknown comparison type\n");
    return -1;
  }

  char left[1024];
  strcpy(left,comparison);
  left[relation-comparison]=0;  
  right=&strchr(&relation[1],'_')[1];

  //  printf("left='%s', right='%s', op=%d\n",left,right,op);

  int is_signed;

  if (op!=GT) {
    is_signed=generate_assignment(right,left,op,destination);
  } else {
    // GT we implement more or less as LE
    is_signed=generate_assignment(left,right,op,destination);
  }
  

  switch(op) {
  case GE:
    if (is_signed) {
      printf("bvc !+\n");
      printf("eor #$80\n");
      printf("!:\n");
      printf("bpl {%s}\n",destination);
    } else {
      printf("bcs {%s}\n",destination);
      printf("!:\n");      
    }
    break;
  case LE:
    if (is_signed) {
    } else {
      printf("beq {%s}\n",destination);
      printf("!:\n");      
      printf("bcc {%s}\n",destination);
    }
    break;
  case EQ:
    printf("beq {%s}\n!:\n",destination);
    break;
  case GT:
    printf("bcc {%s}\n!:\n",destination);
    break;
  case LT:
    printf("bcc {%s}\n!:\n",destination);
    break;
  }
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
    return generate_assignment(left,right,0,NULL);
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
