/**
 *  Copyright (c) 2016 AlfaLoop Technology Co., Ltd. All Rights Reserved.
 *
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential.
 *
 *  Attribution — You must give appropriate credit, provide a link to the license, and
 *  indicate if changes were made. You may do so in any reasonable manner, but not in any
 *  way that suggests the licensor endorses you or your use.
 *
 *  NonCommercial — You may not use the material for commercial purposes under unauthorized.
 *
 *  NoDerivatives — If you remix, transform, or build upon the material, you may not
 *  distribute the modified material.
 */
#include "symtab.h"
#include <string.h>
#include "libs/util/linklist.h"

/*---------------------------------------------------------------------------*/
#if defined(DEBUG_ENABLE)
#define DEBUG_MODULE 0
#if DEBUG_MODULE
#include "dev/syslog.h"
#define PRINTF(...) syslog(__VA_ARGS__)
#else
#define PRINTF(...)
#endif  /* DEBUG_MODULE */
#else
#define PRINTF(...)
#endif  /* DEBUG_ENABLE */
/*---------------------------------------------------------------------------*/
LIST(symtab_list);

/* Binary search is twice as large but still small. */
#ifndef SYMTAB_CONF_BINARY_SEARCH
#define SYMTAB_CONF_BINARY_SEARCH 0
#endif

/*---------------------------------------------------------------------------*/
#if SYMTAB_CONF_BINARY_SEARCH
void *
symtab_lookup(const char *name)
{
  int start, middle, end;
  int r;

  start = 0;
  end = symbols_nelts - 1;	/* Last entry is { 0, 0 }. */

  while(start <= end) {
    /* Check middle, divide */
    middle = (start + end) / 2;
    r = strcmp(name, symbols[middle].name);
    if(r < 0) {
      end = middle - 1;
    } else if(r > 0) {
      start = middle + 1;
    } else {
      return symbols[middle].value;
    }
  }
  return NULL;
}
#else /* SYMTAB_CONF_BINARY_SEARCH */
void *
symtab_lookup(const char *name)
{
 /* const struct symbols *s;
  for(s = symbols; s->name != NULL; ++s) {
    if(strcmp(name, s->name) == 0) {
      return s->value;
    }
  }
  return 0;
  */
	struct symbols *s;
  if (strlen(name) > 2) {
  //  if ( (name[0] == 'A' && name[1] == 'L') ||
  //       (name[0] == 'O' && name[1] == 'S') ) {
    	for(s = list_head(symtab_list);
    		s != NULL;
    		s = list_item_next(s))
    	{
    		PRINTF("symbol %s \n", s->name);
        if (strlen(name) == strlen(s->name)) {
          if(strcmp(name, s->name) == 0) {
      			PRINTF("match symbol %s \n", s->name);
      			return s->value;
      		}
        }
      }
  //  }
	}
	return 0;
}
#endif /* SYMTAB_CONF_BINARY_SEARCH */
/*---------------------------------------------------------------------------*/
void
symtab_init()
{
	list_init(symtab_list);
}
/*---------------------------------------------------------------------------*/
void symtab_add(struct symbols *symb)
{
	list_add(symtab_list, symb);
}
/*---------------------------------------------------------------------------*/
