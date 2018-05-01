/*
 * Copyright (c) 2009, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
/**
 * © Copyright AlfaLoop Technology Co., Ltd. 2018
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/
#include <stdint.h>
#include "contiki-conf.h"
#include "loader/elfloader-arch.h"
#include "spiffs/spiffs.h"

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

#define ELF32_R_TYPE(info)      ((unsigned char)(info))

/* Supported relocations */
#define R_ARM_ABS32				2
#define R_ARM_THM_CALL		10

//static const uint32_t textmemory[ELFLOADER_TEXTMEMORY_SIZE] = {0};
//static uint32_t textmemory[ELFLOADER_TEXTMEMORY_SIZE] = {0};
static const uint32_t textmemory[(ELFLOADER_TEXTMEMORY_SIZE+3)/4]  __attribute__ ((section(".elf_text")))
																				= {0};
/* Adapted from elfloader-arm.c */
static uint32_t datamemory_aligned[(ELFLOADER_DATAMEMORY_SIZE+3)/4]; //word aligned
static uint8_t* datamemory = (uint8_t *)datamemory_aligned;
#define READSIZE sizeof(datamemory_aligned)

/*---------------------------------------------------------------------------*/
void *
elfloader_arch_allocate_ram(int size)
{
	memset(datamemory_aligned , 0x00, (ELFLOADER_DATAMEMORY_SIZE+3)/4);
	PRINTF("[elfloader_arch] allocate ram %lu\n", size);
	if(size > sizeof(datamemory_aligned)){
		PRINTF("[elfloader_arch] reserved ram small\n");
	}
	return datamemory;
}
/*---------------------------------------------------------------------------*/
void *
elfloader_arch_allocate_rom(int size)
{
	memset(textmemory , 0x00, ELFLOADER_TEXTMEMORY_SIZE);
	PRINTF("[elfloader_arch] allocate rom %lu\n", size);
	if(size > sizeof(textmemory)){
		PRINTF("[elfloader_arch] reserved rom small\n");
	}
	return (void *)textmemory;
}
/*---------------------------------------------------------------------------*/
void
elfloader_arch_write_rom(int fd, unsigned short textoff, unsigned int size, char *mem)
{
	uint32_t ptr;
	int nbytes;
	int ret;

  PRINTF("[elfloader_arch] write rom: size %d, offset %i, mem %p READSIZE %d\n", size, textoff, mem, READSIZE);
	SPIFFS_lseek(&SYSFS, fd, textoff, SPIFFS_SEEK_SET);
	for(ptr = 0; ptr < size; ptr += READSIZE) {
		// Read data from file into RAM.
		nbytes = SPIFFS_read(&SYSFS, fd, (unsigned char *)datamemory, READSIZE);
		// Write data to flash.
		// backup the flash content and
		nrf_flash_erase_and_write((uint32_t) mem, datamemory, nbytes);
	}
}
/*---------------------------------------------------------------------------*/
void
elfloader_arch_relocate(int fd, unsigned int sectionoffset,
			char *sectionaddr,
			struct elf32_rela *rela, char *addr)
{
  PRINTF("[elfloader_arch] relocate sectionoffset 0x%04x, sectionaddr %p, r_offset 0x%04x, r_info 0x%04x, r_addend 0x%04x, addr %p\n",
	 sectionoffset, sectionaddr,
	 (unsigned int)rela->r_offset, (unsigned int)rela->r_info,
	 (unsigned int)rela->r_addend, addr);
  unsigned int type;
  type = ELF32_R_TYPE(rela->r_info);
	SPIFFS_lseek(&SYSFS, fd, sectionoffset + rela->r_offset, SPIFFS_SEEK_SET);
  switch(type) {
  case R_ARM_ABS32: // Type 2
    {
			if (rela->r_addend <= 0x2000) {
				int32_t addend;
				SPIFFS_read(&SYSFS, fd, (char*)&addend, 4);
				PRINTF("[elfloader_arch] R_ARM_ABS32 addend: %d \n",addend);
				addr += addend;
				// Update location address
				SPIFFS_lseek(&SYSFS, fd, -4, SPIFFS_SEEK_CUR);
				SPIFFS_write(&SYSFS, fd, &addr, 4);
			  PRINTF("[elfloader_arch] R_ARM_ABS32 write: %p \n",addr);
			}
    }
    break;
  case R_ARM_THM_CALL: // Type 10
    {
      uint16_t instr[2];
      int32_t offset;
      char *base;
			SPIFFS_read(&SYSFS, fd, (char*)instr, 4);
			SPIFFS_lseek(&SYSFS, fd, -4, SPIFFS_SEEK_CUR);
	  	PRINTF("[elfloader_arch] R_ARM_THM_CALL instr[0]=%04x instr[1]=%04x \n", instr[0], instr[1]);

		  // Ignore the addend since it will be zero for calls to symbols,
		  // and I can't think of a case when doing a relative call to
		  // a non-symbol position
      base = sectionaddr + (rela->r_offset + 4);
	  	PRINTF("[elfloader_arch] R_ARM_THM_CALL base addr=%08x \n", base);

      if(((instr[1]) & 0xe800) == 0xe800) {
        /* BL or BLX */
        if(((uint32_t) addr) & 0x1) {
          /* BL */
          instr[1] |= 0x1800;
        } else {
#if defined(__ARM_ARCH_4T__)
          return ELFLOADER_UNHANDLED_RELOC;
#else
          /* BLX */
          instr[1] &= ~0x1800;
          instr[1] |= 0x0800;
#endif
        }
      }

      /* Adjust address for BLX */
      if((instr[1] & 0x1800) == 0x0800) {
        addr = (char *)((((uint32_t) addr) & 0xfffffffd) |
                (((uint32_t) base) & 0x00000002));
      }
      offset = addr - (sectionaddr + (rela->r_offset + 4));
      PRINTF("[elfloader_arch]  offset %d\n", (int)offset);
      if(offset < -(1 << 22) || offset >= (1 << 22)) {
        PRINTF("[elfloader_arch] offset %d too large for relative call\n",
               (int)offset);
      }
      PRINTF("%p: %04x %04x  offset: %d addr: %p\n", sectionaddr +rela->r_offset, instr[0], instr[1], (int)offset, addr);
      instr[0] = (instr[0] & 0xf800) | ((offset>>12)&0x07ff);
      instr[1] = (instr[1] & 0xf800) | ((offset>>1)&0x07ff);

			SPIFFS_write(&SYSFS, fd, &instr, 4);
      PRINTF("[elfloader_arch] write: %04x %04x\n",instr[0], instr[1]);
    }
    break;

  default:
    PRINTF("[elfloader_arch] unsupported relocation type %d\n", type);
  }
}
/*---------------------------------------------------------------------------*/
