#include <stdio.h>
#include "fond/linked-list.h"
#include "fond/linked-list-tree.h"
#include "fond/symbol.h"
#include "fond/string.h"

#if !defined(token_already_included)
#  include "comm/lgram/y.tab.h"
#endif

#include "fond/value.h"

List *lgram_read_file(char *filename);
List *lgram_read_stream(FILE *f);
List *lgram_read_string(char *str);
