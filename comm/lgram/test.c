#include <stdio.h>
#include <unistd.h>
#include "comm/lgram/lgram.h"

extern int yydebug, optind;
extern FILE *stdin;
extern char *optarg;

int main(int argc, char **argv) {
  int  c;
  char *file, *in_str = 0;
  List *res;
  FILE *in;

  while ((c = getopt(argc, argv, "hde:")) != -1) {
    switch (c) {
    default:
    case 'h':
    help:
      fprintf(stderr, "\
Usage: %s [options] [ file | -e string ]\n
Options:\n
-h	Print this message and exit\n\
-d	debug parser\n\
-e      parse commandline
",  argv[0]);
      exit(1);
    case 'd':
      yydebug = 1;
      break;
    case 'e':
      in_str = optarg;
      break;
    }
  }

  if (in_str == 0) {
    if (optind+1 != argc) 
      goto help;
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
  }

  printf("in_str has %s\n", in_str);

  sym_init();

  if (in_str == 0)
    res = lgram_read_stream(in);
  else
    res = lgram_read_string(in_str);

  if ( res != NULL)
    tree_dump(res);

  puts("");

  return(0);
}

