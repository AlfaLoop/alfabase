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
#ifndef ELFLOADER_H_
#define ELFLOADER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "contiki.h"

#define ELFLOADER_SYMBOL_NAME_LENGTH  64

/**
 * Return value from elfloader_load() indicating that loading worked.
 */
#define ELFLOADER_OK                  0
/**
 * Return value from elfloader_load() indicating that the ELF file had
 * a bad header.
 */
#define ELFLOADER_BAD_ELF_HEADER      1
/**
 * Return value from elfloader_load() indicating that no symbol table
 * could be found in the ELF file.
 */
#define ELFLOADER_NO_SYMTAB           2
/**
 * Return value from elfloader_load() indicating that no string table
 * could be found in the ELF file.
 */
#define ELFLOADER_NO_STRTAB           3
/**
 * Return value from elfloader_load() indicating that the size of the
 * .text segment was zero.
 */
#define ELFLOADER_NO_TEXT             4
/**
 * Return value from elfloader_load() indicating that a symbol
 * specific symbol could not be found.
 *
 * If this value is returned from elfloader_load(), the symbol has
 * been copied into the elfloader_unknown[] array.
 */
#define ELFLOADER_SYMBOL_NOT_FOUND    5
/**
 * Return value from elfloader_load() indicating that one of the
 * required segments (.data, .bss, or .text) could not be found.
 */
#define ELFLOADER_SEGMENT_NOT_FOUND   6
/**
 * Return value from elfloader_load() indicating that no starting
 * point could be found in the loaded module.
 */
#define ELFLOADER_NO_STARTPOINT       7


process_event_t PROCESS_ELFLOADER_EVENT;

/**
 * elfloader initialization function.
 *
 * This function should be called at boot up to initialize the elfloader.
 */
void elfloader_init(void);
void elfloader_init_process(void);


/**
 * \brief      Load and relocate an ELF file.
 * \param fd   An open CFS file descriptor.
 * \return     ELFLOADER_OK if loading and relocation worked.
 *             Otherwise an error value.
 *
 *             This function loads and relocates an ELF file. The ELF
 *             file must have been opened with cfs_open() prior to
 *             calling this function.
 *
 *             If the function is able to load the ELF file, a pointer
 *             to the process structure in the model is stored in the
 *             elfloader_loaded_process variable.
 *
 * \note       This function modifies the ELF file opened with cfs_open()!
 *             If the contents of the file is required to be intact,
 *             the file must be backed up first.
 *
 */
int elfloader_load(int fd);


/**
 * A pointer to the processes loaded with elfloader_load().
 */
extern struct process * const * elfloader_autostart_processes;

/**
 * If elfloader_load() could not find a specific symbol, it is copied
 * into this array.
 */
extern char elfloader_unknown[ELFLOADER_SYMBOL_NAME_LENGTH];


typedef unsigned long  elf32_word;
typedef   signed long  elf32_sword;
typedef unsigned short elf32_half;
typedef unsigned long  elf32_off;
typedef unsigned long  elf32_addr;

struct elf32_rela {
  elf32_addr      r_offset;       /* Location to be relocated. */
  elf32_word      r_info;         /* Relocation type and symbol index. */
  elf32_sword     r_addend;       /* Addend. */
};

#ifdef __cplusplus
}
#endif

#endif /* ELFLOADER_H_ */

/** @} */
/** @} */
