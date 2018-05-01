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
#ifndef ELFLOADER_ARCH_H_
#define ELFLOADER_ARCH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "loader/elfloader.h"

/**
 * \brief      Allocate RAM for a new module.
 * \param size The size of the requested memory.
 * \return     A pointer to the allocated RAM
 *
 *             This function is called from the Contiki ELF loader to
 *             allocate RAM for the module to be loaded into.
 *
 * \bug        The Contiki ELF loader currently does not contain a
 *             mechanism for deallocating the memory allocated with
 *             this function.
 */
void *elfloader_arch_allocate_ram(int size);

/**
 * \brief      Allocate program memory for a new module.
 * \param size The size of the requested memory.
 * \return     A pointer to the allocated program memory
 *
 *             This function is called from the Contiki ELF loader to
 *             allocate program memory (typically ROM) for the module
 *             to be loaded into.
 *
 * \bug        The Contiki ELF loader currently does not contain a
 *             mechanism for deallocating the memory allocated with
 *             this function.
 */
void *elfloader_arch_allocate_rom(int size);

/**
 * \brief      Perform a relocation.
 * \param fd   The file descriptor for the ELF file.
 * \param sectionoffset The file offset at which the relocation can be found.
 * \param sectionaddr The section start address (absolute runtime).
 * \param rela A pointer to an ELF32 rela structure (struct elf32_rela).
 * \param addr The relocated address.
 *
 *             This function is called from the Contiki ELF loader to
 *             perform a relocation on a piece of code or data. The
 *             relocated address is calculated by the Contiki ELF
 *             loader, based on information in the ELF file, and it is
 *             the responsibility of this function to patch the
 *             executable code. The Contiki ELF loader passes a
 *             pointer to an ELF32 rela structure (struct elf32_rela)
 *             that contains information about how to patch the
 *             code. This information is different from processor to
 *             processor.
 */
void elfloader_arch_relocate(int fd, unsigned int sectionoffset,
			     char *sectionaddr,
			     struct elf32_rela *rela, char *addr);

/**
 * \brief      Write to read-only memory (for example the text segment).
 * \param fd   The file descriptor for the ELF file.
 * \param textoff	Offset of text segment relative start of file.
 * \param size The size of the text segment.
 * \param mem  A pointer to the where the text segment should be flashed
 *
 *             This function is called from the Contiki ELF loader to
 *             write the program code (text segment) of a loaded
 *             module into memory. The function is called when all
 *             relocations have been performed.
 */
void elfloader_arch_write_rom(int fd, unsigned short textoff, unsigned int size, char *mem);

#ifdef __cplusplus
}
#endif

#endif /* ELFLOADER_ARCH_H_ */

/** @} */
/** @} */
