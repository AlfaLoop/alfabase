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
#include "spiffs/spiffs.h"
#include "contiki-conf.h"
#include <stdbool.h>
#include <stdint.h>
#include "dev/watchdog.h"
#include "dev/lpm.h"
#include "nrf_soc.h"
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "ble_radio_notification.h"
#include "nrf_error.h"
#include "nest-driver.h"
#include "bsp_init.h"

#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#endif

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
spiffs nrf_spiffs;
static u8_t spiffs_work_buf[FLASH_PAGE_SIZE];   //4096
#if USE_SPIFFS_CACHE == 1
static u8_t spiffs_cache_buf[FLASH_PAGE_SIZE / 4];  //1024
#endif
static u8_t spiffs_fds[32 * 4];  // 128
// static u8_t tmpbuffer[256]; // 256
static u32_t tmpbuffer[256];
static uint8_t flash_accessing = 0;

#ifdef USE_ELFLOADER
static uint32_t tmpbuf[FLASH_PAGE_SIZE / 4];
#endif
/*--------------------------------------------------------------------------*/
uint8_t
is_fs_arch_processing(void)
{
	return flash_accessing;
}
/*--------------------------------------------------------------------------*/
static void
sys_evt_dispatch(uint32_t sys_evt)
{
	PRINTF("[nrf spiffs] sys evt dispatch \n");
	flash_accessing = false;
	watchdog_periodic();
}
/*--------------------------------------------------------------------------*/
static void
erase_flash_in_fs_area(void)
{
	PRINTF("[nrf spiffs] erase flash in fs area\n");

	uint32_t ret;
	uint32_t addr = NRF_FLASH_MAP_FS_START;
	uint32_t end = addr + NRF_FLASH_MAP_FS_SIZE;
	uint32_t sector;
	do {
		sector = addr / FLASH_PAGE_SIZE;
		PRINTF("[nrf spiffs] erase sector %d 0x%08X\n", sector, sector * 4096);

		// Do nothing (just wait for radio to become inactive).
		while (is_radio_accessing()){
			watchdog_periodic();
			lpm_drop();
		}
		watchdog_periodic();
		flash_accessing = 1;
		ret = sd_flash_page_erase(sector);
		if (ret == NRF_SUCCESS || ret == NRF_ERROR_BUSY) {
			while (flash_accessing){
				lpm_drop();
			}
		} else {
			PRINTF("[nrf spiffs] sd flash page erase error %d\n", ret);
		}
		watchdog_periodic();
		addr += FLASH_PAGE_SIZE;
	} while (addr < end);
}
/*---------------------------------------------------------------------------*/
static s32_t
nrf_spiffs_read(struct spiffs_t *fs, u32_t addr, u32_t size, u8_t *dst)
{
	nrf_spiffs_flash_read(addr, dst, size);
	return SPIFFS_OK;
}
/*---------------------------------------------------------------------------*/
static s32_t
nrf_spiffs_write(struct spiffs_t *fs, u32_t addr, u32_t size, u8_t *src)
{
	nrf_spiffs_flash_write(addr, src, size);
	return SPIFFS_OK;
}
/*---------------------------------------------------------------------------*/
static s32_t
nrf_spiffs_erase(struct spiffs_t *fs, u32_t addr, u32_t size)
{
	watchdog_periodic();
	nrf_spiffs_flash_erase(addr, size);
	return SPIFFS_OK;
}
/*---------------------------------------------------------------------------*/
static void
nrf_spiffs_arch_format(void)
{
	erase_flash_in_fs_area();
	SPIFFS_format(&nrf_spiffs);
}
/*---------------------------------------------------------------------------*/
static int
nrf_spiffs_arch_mount(void)
{
	// initialize spiffs
	spiffs_config cfg;
	cfg.phys_addr = NRF_FLASH_MAP_FS_START;
	cfg.phys_size = NRF_FLASH_MAP_FS_SIZE;
	cfg.phys_erase_block = 4096;
	cfg.log_block_size = 4096;
	cfg.log_page_size = 256;
	cfg.hal_read_f = nrf_spiffs_read;
	cfg.hal_write_f = nrf_spiffs_write;
	cfg.hal_erase_f = nrf_spiffs_erase;
#if USE_SPIFFS_CACHE == 1
	return SPIFFS_mount(&nrf_spiffs,
		&cfg,
		spiffs_work_buf,
		spiffs_fds,
		sizeof(spiffs_fds),
		spiffs_cache_buf,
		sizeof(spiffs_cache_buf),
		0
	);
#else
	return SPIFFS_mount(&nrf_spiffs,
		&cfg,
		spiffs_work_buf,
		spiffs_fds,
		sizeof(spiffs_fds),
		NULL,
		0,
		0
	);
#endif

}
/*---------------------------------------------------------------------------*/
#ifdef USE_ELFLOADER
void
nrf_flash_erase_and_write(uint32_t address, const void *data, uint32_t length)
{
	const uint32_t end = address + length;
	uint32_t err_code;
	uint32_t i,j;
	uint32_t retval;
	uint32_t next_page_addr, curr_page_addr;
	uint16_t offset;
	uint8_t *p_buf = (uint8_t*)tmpbuf;
	uint8_t erase_need;

	PRINTF("[nrf spiffs] write address %d length %d\n", address, length);
	for(i = address; i < end;) {
		next_page_addr = (i | (FLASH_PAGE_SIZE - 1)) + 1;
		curr_page_addr = i & ~(FLASH_PAGE_SIZE - 1);
		offset = i - curr_page_addr;
		if(next_page_addr > end) {
			next_page_addr = end;
		}
		erase_need = 0;
		watchdog_periodic();
		/* Read a page from flash and put it into a mirror buffer. */
		nrf_spiffs_flash_read(curr_page_addr, p_buf, FLASH_PAGE_SIZE);

		/* Update flash mirror data with new data. */
		memcpy(p_buf + offset, data, next_page_addr - i);

		watchdog_periodic();
		while (is_radio_accessing()){
			// Do nothing (just wait for radio to become inactive).
			lpm_drop();
			watchdog_periodic();
		}
		watchdog_periodic();

		flash_accessing = 1;
		watchdog_periodic();
		retval = sd_flash_page_erase(curr_page_addr / FLASH_PAGE_SIZE);
		if (retval == NRF_SUCCESS || retval == NRF_ERROR_BUSY) {
			while (flash_accessing){
				lpm_drop();
			}
		} else {
			PRINTF("[nrf spiffs] sd_flash_page_erase %d\n", retval);
		}
		// If radio is active, wait for it to become inactive.
		while (is_radio_accessing()){
			lpm_drop();
		}
		watchdog_periodic();
		flash_accessing = 1;
		retval = sd_flash_write((uint32_t *)curr_page_addr,
								(uint32_t *)tmpbuf,
								FLASH_PAGE_SIZE/ 4);
		watchdog_periodic();
		if (retval == NRF_SUCCESS || retval == NRF_ERROR_BUSY) {
			while (flash_accessing){
				lpm_drop();
			}
		} else {
			PRINTF("[nrf spiffs] sd_flash_write %d\n", retval);
		}
		watchdog_periodic();
		// Write modified data form mirror buffer into the flash.
		data = (uint8_t *) data + next_page_addr - i;
		i = next_page_addr;
	}
}
#endif
/*---------------------------------------------------------------------------*/
void
nrf_spiffs_flash_read(uint32_t addr, uint8_t *data, uint32_t size)
{
	//PRINTF("[nrf spiffs] spiffs read addr %d size %d\n", addr, size);
	uint8_t *p_data = (uint8_t *)data;
	memcpy((uint8_t *)data, (uint8_t *)addr , size);
}
/*---------------------------------------------------------------------------*/
void
nrf_spiffs_flash_write(uint32_t addr, uint8_t *data, uint32_t size)
{
#if DEBUG_MODULE > 0
	PRINTF("[nrf spiffs] nrf_spiffs_flash_write 0x%08x size %d\n", addr, size);
#endif
	static uint32_t curr_page_addr;
	uint32_t retval;
	uint32_t end = addr + size;

	uint32_t addr_offset = 0;
	uint8_t *p_buf = (uint8_t*)tmpbuffer;

	if (addr % 4 != 0) {
		addr_offset = addr % 4;
		addr = addr - (addr % 4);
	}

	if (end % 4 != 0) {
		end = (end + 4) - (end % 4);
	}
	watchdog_periodic();
	// nrf_spiffs_flash_read(addr, tmpbuffer, end - addr);
	nrf_spiffs_flash_read(addr, p_buf, end - addr);

	memcpy(p_buf + addr_offset, data, size);
	size = end - addr;


	// If radio is active, wait for it to become inactive.
	while (is_radio_accessing() || flash_accessing)	{
		 lpm_drop();
	}
	watchdog_periodic();

	flash_accessing = 1;
	retval = sd_flash_write((uint32_t *)addr,
							(uint32_t *)tmpbuffer,
							size / sizeof(uint32_t) );
	if (retval == NRF_SUCCESS || retval == NRF_ERROR_BUSY) {
		while (is_radio_accessing() || flash_accessing)	{
			 PRINTF("[nrf spiffs] lpm_drop %d\n", flash_accessing);
			 lpm_drop();
		}
	} else {
		PRINTF("[nrf spiffs] sd_flash_write %d\n", retval);
	}
	watchdog_periodic();
}
/*---------------------------------------------------------------------------*/
void
nrf_spiffs_flash_erase(uint32_t addr, uint32_t size)
{
	uint32_t ret;
	uint32_t end = addr + size;
	uint32_t sector;
	PRINTF("[nrf spiffs] nrf_spiffs_flash_erase \n");

	// Make sure the sector is in range of COFFEE
	/*if (( addr < FLASH_MAP_FS_START) ||
		( end >= (FLASH_MAP_FS_START + FLASH_MAP_FS_SIZE)) )
	{
		PRINTF("[nrf spiffs] erase out of range \n");
		return;
	}*/

	do {
		sector = addr / FLASH_PAGE_SIZE;
		PRINTF("[nrf spiffs] nrf spiifs flash erase sector %d addr %d\n", sector, addr);

		while (is_radio_accessing()) {
			lpm_drop();
		}
		flash_accessing = 1;
		ret = sd_flash_page_erase(sector);
		if (ret == NRF_SUCCESS || ret == NRF_ERROR_BUSY) {
			while (flash_accessing){
				lpm_drop();
			}
		} else {
			PRINTF("[nrf spiffs] busy %d\n", ret);
		}
		watchdog_periodic();
		addr += FLASH_PAGE_SIZE;
	} while (addr < end);
}
/*---------------------------------------------------------------------------*/
int
nrf_spiffs_arch_init(uint8_t format)
{
	int ret;

	// Register with the SoftDevice handler module for BLE events.
	softdevice_sys_evt_handler_set(sys_evt_dispatch);

	// mount filesystem
	if (!format) {
		PRINTF("[nrf spiffs] force format filesystem........\n");
		nrf_spiffs_arch_format();
		ret = nrf_spiffs_arch_mount();
	} else {
		ret = nrf_spiffs_arch_mount();
		if (ret < 0 ){
			PRINTF("[nrf spiffs] spiffs mount error.. format!\n");
			nrf_spiffs_arch_format();
			ret = nrf_spiffs_arch_mount();
		}
	}
	PRINTF("[nrf spiffs] spiffs mount completed\n");
	return ret;
}
/*---------------------------------------------------------------------------*/
