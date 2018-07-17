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
#include "spiffs-flash-arch.h"
#include "contiki-conf.h"
#include "spiffs/spiffs.h"
#include <stdbool.h>
#include <stdint.h>
#include "dev/watchdog.h"
#include "dev/spi.h"
#include "errno.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "app_util_platform.h"
#include "nrf_soc.h"
/*---------------------------------------------------------------------------*/
#if defined(DEBUG_ENABLE)
#define DEBUG_MODULE 1
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
#define SYSC_FS_SIZE            STORAGE_SYSC_FS_FLSAH_SIZE_CONF
#define SYSC_FS_PAGES            STORAGE_SYSC_FS_FLSAH_PAGES_CONF
#define SYSC_FS_PAGE_SIZE        STORAGE_SYSC_FS_FLSAH_PAGE_SIZE_CONF
#define SYSC_FS_SECTOR_SIZE      STORAGE_SYSC_FS_FLSAH_SECTOR_SIZE_CONF

#if defined(SPI_FLASH_DRIVER_CONF)
#define SPI SPI_FLASH_DRIVER_CONF
#else
#error Need define SPI_FLASH_DRIVER_CONF driver instance
#endif
extern const struct spi_driver SPI;

/*--------------------------------------------------------------------------*/
spiffs extfs;
static u8_t spiffs_work_buf[SYSC_FS_PAGE_SIZE * 2];
static u8_t spiffs_cache_buf[SYSC_FS_PAGE_SIZE];
static u8_t spiffs_fds[32 * 4];
/*--------------------------------------------------------------------------*/
static void
w25q_flash_read_status(uint8_t *status)
{
  uint8_t command[] = {0x05};
	SPI.enable();
	nrf_gpio_pin_clear(SPI_FLASH_CS);
  SPI.transfer(command, 1, NULL, 0);
	SPI.transfer(NULL, 0, status, 1);
	nrf_gpio_pin_set(SPI_FLASH_CS);
	SPI.disable();
}
/*--------------------------------------------------------------------------*/
static bool
w25q_flash_write_enable(void)
{
	uint32_t errcode;
	uint8_t command[] = {0x06};
	SPI.enable();
	nrf_gpio_pin_clear(SPI_FLASH_CS);
	errcode = SPI.transfer(command, 1, NULL, 0);
	nrf_gpio_pin_set(SPI_FLASH_CS);
	SPI.disable();
	if (errcode != ENONE)
		return false;
	return true;
}
/*--------------------------------------------------------------------------*/
static bool
w25q_flash_wait_until_ready() {
  uint8_t status = 0;
  w25q_flash_read_status(&status);
  watchdog_periodic();
  if (status & 0x01 == 0) {
       return true;
  }
  // first wait 10us, second 100us, third and following 1000us
  uint32_t wait_time = 10;
  do {
      nrf_delay_us(wait_time); //wait 1 ms
      w25q_flash_read_status(&status);
      if (wait_time < 1000) {
          wait_time *= 10;
      }
    //  PRINTF("w25q wait.....%d\n", status);
  } while (status & 0x01);
//  PRINTF("w25q statu.....%d\n", status);
  return true;
}
/*--------------------------------------------------------------------------*/
static bool
w25q_flash_write_page(uint32_t addr, uint8_t *data, uint32_t length)
{
  bool success = w25q_flash_write_enable();
	if (success) {
		uint8_t command[] = {0x02, 0xFF, 0xFF, 0xFF};
		command[1] = addr >> 16 & 0xFF;
		command[2] = addr >> 8 & 0xFF;
		command[3] = addr & 0xFF;
		SPI.enable();
		nrf_gpio_pin_clear(SPI_FLASH_CS);
		SPI.transfer(command, 4, NULL, 0);
		SPI.transfer(data, length, NULL, 0);
    watchdog_periodic();
		nrf_gpio_pin_set(SPI_FLASH_CS);
		SPI.disable();
	}
	if (success) {
		success = w25q_flash_wait_until_ready();
	}
	return success;
}
/*--------------------------------------------------------------------------*/
static bool
w25q_flash_erase_sector(uint32_t addr)
{
	uint32_t errcode;
	uint8_t command[] = {0x20, 0xFF, 0xFF, 0x00};
	bool success = w25q_flash_write_enable();

	if (success) {
		//PRINTF("flash_erase_sector %d", addr);
		command[1] = addr >> 16 & 0xFF;
		command[2] = addr >> 8 & 0xF0;

		SPI.enable();
		nrf_gpio_pin_clear(SPI_FLASH_CS);
		errcode = SPI.transfer(command, 4, NULL, 0);
		nrf_gpio_pin_set(SPI_FLASH_CS);
		SPI.disable();
		if (errcode != ENONE) {
			success = false;
			PRINTF("ERROR Code %d", errcode);
		}	else {
			success = true;
		}
	}
	if (success) {
    success = w25q_flash_wait_until_ready();
  }
  watchdog_periodic();
	return success;
}
/*---------------------------------------------------------------------------*/
static bool
w25q_flash_erase_chip(void)
{
	uint32_t errcode;
  watchdog_periodic();
  bool success = w25q_flash_write_enable();
  PRINTF("w25q erase chip\n");
  if (success) {
    uint8_t command[] = {0xC7};
  //  uint8_t command[] = {0x60};
  	SPI.enable();
  	nrf_gpio_pin_clear(SPI_FLASH_CS);
  	errcode = SPI.transfer(command, 1, NULL, 0);
  	nrf_gpio_pin_set(SPI_FLASH_CS);
  	SPI.disable();
  	if (errcode != ENONE) {
  		success = false;
  		PRINTF("ERROR Code %d", errcode);
  	}
  	else {
  		success = true;
  	}
  }

  if (success) {
      success = w25q_flash_wait_until_ready();
  }
  PRINTF("w25q success erase chip\n");

  watchdog_periodic();
  return success;
}

/*---------------------------------------------------------------------------*/
static s32_t
spiffs_w25q_read(struct spiffs_t *fs, u32_t addr, u32_t size, u8_t *dst)
{
	uint32_t errcode;
	uint8_t    command[] = {0x03, 0xFF, 0xFF, 0xFF};
  command[1] = addr >> 16 & 0xFF;
  command[2] = addr >> 8 & 0xFF;
  command[3] = addr & 0xFF;
  watchdog_periodic();
	SPI.enable();
	nrf_gpio_pin_clear(SPI_FLASH_CS);
	errcode = SPI.transfer(command, 4, NULL, 0);
	errcode = SPI.transfer(NULL, 0, dst, size);
	nrf_gpio_pin_set(SPI_FLASH_CS);
	SPI.disable();

	if (errcode != ENONE) {
		PRINTF("spiffs_w25q_read ERROR: %d\n", errcode);
		return SPIFFS_ERR_INTERNAL;
	}
	return SPIFFS_OK;
}
/*---------------------------------------------------------------------------*/
static s32_t
spiffs_w25q_write(struct spiffs_t *fs, u32_t addr, u32_t size, u8_t *src)
{
	uint32_t end = addr + size;
  uint32_t first_page_offset = addr & (SYSC_FS_PAGE_SIZE-1);
  uint32_t page_size = (size + first_page_offset > SYSC_FS_PAGE_SIZE) ? SYSC_FS_PAGE_SIZE - first_page_offset : size;
  bool success;
  watchdog_periodic();
  success = w25q_flash_write_page(addr, src, page_size);
  addr = (0xFFFFFF00 & addr) + SYSC_FS_PAGE_SIZE;
  src += page_size;
  while (addr < end) {
    if (!success) {
			PRINTF("spiffs_w25q_write ERROR\n");
			return SPIFFS_ERR_INTERNAL;
    }
    watchdog_periodic();
    page_size = ((end - addr) > SYSC_FS_PAGE_SIZE) ? SYSC_FS_PAGE_SIZE : (end - addr);
    success = w25q_flash_write_page(addr, src, page_size);
    addr += SYSC_FS_PAGE_SIZE;
    src += page_size;
  }
  watchdog_periodic();
	return SPIFFS_OK;
}
/*---------------------------------------------------------------------------*/
static s32_t
spiffs_w25q_erase(struct spiffs_t *fs, u32_t addr, u32_t size)
{
	uint32_t end = addr + size;
  bool success;
  success = w25q_flash_erase_sector(addr);
  watchdog_periodic();
  addr = (0xFFFFF000 & addr) + SYSC_FS_SECTOR_SIZE;

  while (addr < end) {
    if (!success) {
      PRINTF("spiffs_w25q_erase ERROR\n");
      return SPIFFS_ERR_INTERNAL;
    }
    PRINTF("w25q erase %d\n", addr);
    success = w25q_flash_erase_sector(addr);
    addr += SYSC_FS_SECTOR_SIZE;
  }
	return SPIFFS_OK;
}
/*---------------------------------------------------------------------------*/
static void
spiffs_flash_arch_format(void)
{
	w25q_flash_erase_chip();
	SPIFFS_format(&extfs);
}
/*---------------------------------------------------------------------------*/
static int
spiffs_flash_arch_mount(void)
{
	// initialize spiffs
	spiffs_config cfg;
  cfg.phys_addr = (0);
  cfg.phys_size = SYSC_FS_SIZE;
  cfg.phys_erase_block = (4 * 1024);
  cfg.log_block_size = (64 * 1024) ;
  cfg.log_page_size = (256);
	cfg.hal_read_f = spiffs_w25q_read;
	cfg.hal_write_f = spiffs_w25q_write;
	cfg.hal_erase_f = spiffs_w25q_erase;
	return SPIFFS_mount(&extfs,
						&cfg,
						spiffs_work_buf,
						spiffs_fds,
						sizeof(spiffs_fds),
						spiffs_cache_buf,
						sizeof(spiffs_cache_buf),
						0);
}
/*---------------------------------------------------------------------------*/
int
spiffs_flash_arch_init(uint8_t format)
{
	int ret;
  uint32_t errcode;

	nrf_gpio_cfg_output(SPI_FLASH_CS);
	nrf_gpio_pin_set(SPI_FLASH_CS);

	spi_master_config_t config;
	config.freq =  SPI_FREQUENCY_M4;
	config.sck_pin = SPI_FLASH_SCLK;
	config.mosi_pin = SPI_FLASH_MOSI;
	config.miso_pin = SPI_FLASH_MISO;
	config.irq_priority = APP_IRQ_PRIORITY_LOW;
	config.order = SPI_MSB;
	config.mode = SPI_MODE0;
  config.cs_pin = SPI_FLASH_CS;
	SPI.init(&config);

  uint8_t command[4] = {0x90, 0x00, 0x00, 0x00};
  uint8_t result[3] ;
  SPI.enable();
  nrf_gpio_pin_clear(SPI_FLASH_CS);
  errcode = SPI.transfer(&command[0], 4, NULL, 0);
  errcode = SPI.transfer(NULL, 0, result, 2);
  nrf_gpio_pin_set(SPI_FLASH_CS);
  SPI.disable();

  PRINTF("[flash dev] Manufacturer ID %d \n", result[0]);
  PRINTF("[flash dev] Memory Type %d \n", result[1]);
  PRINTF("[flash dev] Capacity %d \n", result[2]);

	//spiffs_flash_arch_format();
  // mount filesystem
  if (!format) {
    PRINTF("[flash dev] force spiffs format\n");
    spiffs_flash_arch_format();

    PRINTF("[flash dev] flash spiffs mount........\n");
    ret = spiffs_flash_arch_mount();
    PRINTF("[flash dev] errno %i  %i\n", SPIFFS_errno(&extfs), ret);
    if (ret == SPIFFS_ERR_NOT_A_FS) {
      SPIFFS_unmount(&extfs);
      SPIFFS_format(&extfs);
      ret = spiffs_flash_arch_mount();
    }
  } else {
    ret = spiffs_flash_arch_mount();
    if (ret < 0 ){
      spiffs_flash_arch_format();
      ret = spiffs_flash_arch_mount();
      PRINTF("[flash dev] errno %i\n", SPIFFS_errno(&extfs));
    }
  }
	return ret;
}
/*---------------------------------------------------------------------------*/
