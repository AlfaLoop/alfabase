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
#ifndef SYMTAB_H_
#define SYMTAB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "loader/symbols.h"

void *symtab_lookup(const char *name);

void symtab_init(void);
void symtab_add(struct symbols *symb);

#ifdef __cplusplus
}
#endif

#endif /* SYMTAB_H_ */
