/* user interface and vm initialisation

  Copyright (C) 2002 Markus Liedl, markus.lado@gmx.de

  This file is part of Carbone.

  This is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "fond/rb.h"
#include "fond/value.h"
#include "fond/allocate.h"
#include "fond/callable.h"
#include "fond/frame.h"

#include "vm/vm.h"
#include "vm/cmcache.h"
#include "comm/lgram/lgram.h"
#include "comp/cfunc.h"
#include "comp/generator.h"
#include "fond/string.h"
#include "fond/code.h"
#include "fond/object.h"
#include "fond/fixnum.h"
#include "fond/const.h"
#include "fond/class.h"
#include "fond/method.h"
#include "fond/block.h"
#include "fond/frame.h"


void load_builtins(int debug_internals, int debug);
void load_bifile(char *filename, int debug);

FILE *vm_out;

int vm_debug;
int vm_profile;

extern int num_of_superinstructions; // from vm/peephole.c

int  statistics = 0;

extern int   optind;
extern char* optarg;
extern FILE* stdin;
int main(int argc, char **argv) {
  List *tree;

  int debug_internals = 0;
  int dump_after_comp = 0;
  int trace_all = 0;
  int  explain_vm = 0;
  char *in_str = (char*)0, c;
  int  dont_expect = 0;
  
  FILE *in = (FILE*)0;
  vm_profile = 0;

  while ((c = getopt(argc, argv, "hctde:piVSs")) != -1) {
    switch (c) {
    default:
    case 'h':
    help:
      fprintf(stderr, "\n\
Usage: %s [options] [ file | -e string ]\n\
Options:\n\
-h      Print this message and exit\n\
-c      dump compiled code before execution\n\
-t      trace execution\n\
-e      use commandline for input\n\
-p      profile VM code sequences >stderr\n\
-i      inspect compilation of vm builtins\n\
-V      version info plus details about VM internals\n\
-S      don't use superinstructions\n\
-s      print statistics\n\
",  argv[0]);
      exit(1);
    case 'c':
      dump_after_comp = 1;
      break;
    case 'p':
      vm_profile = 1;
      break;
    case 'e':
      in_str = optarg;
      goto start;
      break;
    case 't':
      trace_all = 1;
      break;
    case 'i':
      debug_internals = 1;
      break;
    case 'V':
      explain_vm = 1;
      break;
    case 'S':
      use_super = 0;
      break;
    case 's':
      statistics = 1;
      break;
    }
  }

  if (in_str == 0 && optind+1 == argc) {
    char *file;
    file = argv[optind];

    if (strcmp(file, "-") == 0) {
      in = stdin; 
      argv++; argc--;
    } else if (in_str == 0) {
      if ( (in=fopen(file,"r")) == NULL) {
	perror(argv[optind]);
	exit(1);
      }
    }
  } else if (!debug_internals && !explain_vm)
    goto help;

  if (explain_vm) {
    fprintf(stderr, "\n\
Carbone v%s -- A efficient Ruby VM\n\
  # of superinstructions : %i\n\
  class-method cache     : %s\n\
  compiled with          : " COMPILED_WITH_FLAGS "\n\
  engine compiled with   : " ENGINE_COMPILED_WITH_FLAGS "\n\
  %s\n\
",	    VM_VERSION,
	    num_of_superinstructions,
	    cmcache_description,
	    use_super ? "" : "not using superinstructions");
  }
  
 start:

  // vm_debug influences also recompilation
  // and peeptable and profiling
  vm_debug = trace_all || vm_profile;
  if (vm_debug) {
    setbuf(stdout, 0);
    setbuf(stderr, 0);
  }

  alloc_init();
  sym_init();
  gen_init();
  code_init();
  cfunc_init();
  init_vm_desc();
  cmc_init();

  const_init();
  meth_init();
  class_init();
  obj_init();

  ca_init();
  frm_init();

  blk_init();  // uses vmbuiltin_define_method() which depends on class_proc
  fixnum_init();

  load_builtins(debug_internals, dump_after_comp);

  if (trace_all || dump_after_comp)
    printf("initialized\n");

  if (in != (FILE*)0)
    tree = lgram_read_stream(in);
  else if (in_str != (void*)0) { // -e string
    // `(snippets (,arg (nil)))
    tree = list_new2(  intern("snippets"),
		       (VALUE)list_new2(  (VALUE)lgram_read_string(in_str),
					  rb_nil));
    dont_expect = 1;
  }
  else
    return(0);

  if (tree == 0) {
    fprintf(stderr, "parse error.\n");
    exit(1);
  }

  if (car(tree) != intern("snippets")) {
    printf("\n\
You can run ruby-snippets coded in lgram.\n\
They have the form `(snippets ((code-tree) (result)))'\n\
You can put as many code/expected pairs as you want.\n\
");
    exit(1);
  }
  
  {
    List *s = cdr(tree);
    while (s) {
      Code *code;
      Cell exp;
      List *tree = (List*)caar(s);
      Cell res;

      if (dump_after_comp) {
	puts("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -");
	tree_dump(tree);
	puts("-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -");
      }

      code = gen_code(tree, trace_all || vm_profile);
      

      if (dump_after_comp) {
	code_dump_stdout(code);      // disassemble code
	puts("-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -");
      }

      res = code_invoke(code, trace_all, vm_profile);

      if (trace_all || dump_after_comp || in_str)
	printf("\n   ==> 0x%lx (%li)\n", (Cell)res, value2long(res));

      exp = cadar(s);

      if (exp != res && !dont_expect) {

	printf("XXXXXXXXXXXXXXXXXXX   WRONG RESULT:    XXXXXXXXXXXXXXXXXXXXXXXXXX\n"
               "expected: 0x%lx (%li)\n", (Cell)exp, (Cell)exp);
      }
      else
	if (!vm_profile)
	  printf(".");

      s = cdr(s);
    }
  }

  if (vm_profile)
    code_dump_profile();

  if (statistics || counter_class_find_method > 2000)
    printf("uncached method searches: %li\n", counter_class_find_method);

  return(0);
}


char bif[][50] =
{
   "builtins.lg",
   ""
};


// 1. this wont stay like that...
// 2. tis has nothing to do with subdirectory carbone/builtins!

void load_builtins(int debug_internals, int debug) {
  char *root;
  int  i;

  const_set(intern("VM_INTERNAL_Trace"), debug);

  // class Class; def define_method(tree) ... end end

  // this one wouldnt have to be hardcoded

  root =  "(define_method                             \n\
                ((param method-code))                 \n\
                (cfunc meth_generate                  \n\
		  (cfunc meth_from_list               \n\
		    (self)                            \n\
		    (par method-code))                \n\
		  (const (nil) VM_INTERNAL_Trace)))";

  /* we need to call meth_generate, because automatic compilation is
     not yet working
  */
  meth_generate(meth_from_list( class_class,
				(List*)lgram_read_string(root)),
		debug_internals);


  for(i=0; strlen(bif[i])>0; i++) {
    load_bifile(bif[i], debug_internals);
  }
}

void load_bifile(char *filename, int debug) {

  List *l          = lgram_read_file(filename);
  List *lc;
  VALUE classname;
  Class *class;

  //printf("having loaded:\n");
  //tree_dump(l);


  while(l) {
    lc = (List*)car(l);
  
    classname = car(lc);
    lc = cdr(lc);

    if (debug)
      fprintf(stderr, "[%s]", sym2str(classname));
    
    class = (Class*)const_get(classname);
    assert(class->class == class_class);

    while(lc) {
      if (debug) {
	puts("");
	tree_dump((List*)car(lc));
	puts("");
      }
      
      meth_generate(
        meth_from_list(class, (List*)car(lc)),
		    debug);
      
      lc = cdr(lc);
    }
    l = cdr(l);
  }
}
