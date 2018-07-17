/**
 *  Copyright (c) 2016 AlfaLoop Technology Co., Ltd. All Rights Reserved.
 *
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential.
 *
 *  Attribution - You must give appropriate credit, provide a link to the license, and
 *  indicate if changes were made. You may do so in any reasonable manner, but not in any
 *  way that suggests the licensor endorses you or your use.
 *
 *  NonCommercial - You may not use the material for commercial purposes under unauthorized.
 *
 *  NoDerivatives - If you remix, transform, or build upon the material, you may not
 *  distribute the modified material.
 */
#include "contiki.h"
#include "dwt-radio.h"
#include <stdint.h>
#include <stdbool.h>
#include "dev/spi.h"
#include "dev/ext_modules.h"
#include "dev/radio/dwt/dw1000.h"
#include "gpiote.h"
#include "nrf_gpio.h"
#include "app_util_platform.h"
#include "sys/clock.h"
#include "errno.h"
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
/*
  计算公式 antenna_delay = 距离(米) / 4.6917519677e-3
  其中4.6917519677e-3为40位计数器一个周斯内电磁波的传播距离
  RX与TX的Antenna Delay之和为两者测距时的稳态误差

  例如，TX与RX1的测距稳态误差为158米，则两者天线延时之和为：
  158 / 4.691e-3 = 33675
 */
#define ANTENNA_DELAY (16838 - 25)
#define DW1000_DEV_ID           ((uint8_t)0x00)     // Lens = 4,    RO,  Device Identifier – includes device type and revision info

/*---------------------------------------------------------------------------*/
const struct ext_module module_dwt;
/*---------------------------------------------------------------------------*/
extern const struct spi_driver spi1_driver;
//extern const struct spi_driver DW_SPI_DRIVER;
/*---------------------------------------------------------------------------*/
static void DW1000_Delay( void )
{
  uint32_t count = 32;
  while(count--);
}
/*---------------------------------------------------------------------------*/
static void
DW1000_readData( uint16_t regAddr, uint16_t subIndex, uint8_t *readBuf, uint32_t lens )
{
  uint8_t header[3] = {0};
  uint32_t count = 0;

  if(subIndex == 0) {     // sub-index is not present
    header[count++] = (uint8_t)regAddr;
  }
  else {                  // sub-index is present
    header[count++] = (uint8_t)(0x40 | regAddr);
    if(subIndex <= 0x7F)  // 7-bit,  subIndex <= 0x7F
      header[count++] = (uint8_t)subIndex;
    else {                // 15-bit, subIndex <= 0x7FFF, extended address
      header[count++] = 0x80 | (uint8_t)subIndex;
      header[count++] = (uint8_t)(subIndex >> 7);
    }
  }

  #if DEBUG_MODULE > 1
    int i;
    PRINTF("spiWrite header length %d\n", count);
    for ( i = 0; i < count; i++) {
      PRINTF("%d ", header[i]);
    }
  #endif


  spi1_driver.enable();
  nrf_gpio_pin_clear(SPI_DWT_CS);

  spi1_driver.transfer(header, count, NULL, 0);
  spi1_driver.transfer(NULL, 0, readBuf, lens);
  // Read from SPI
  /*SPIx_CSD_L();
  for(uint8_t i = 0; i < count; i++)
    SPI_RW(SPIx, header[i]);
  for(uint8_t i = 0; i < lens; i++)
    readBuf[i] = SPI_RW(SPIx, 0x00);
  SPIx_CSD_H();
*/
  nrf_gpio_pin_set(SPI_DWT_CS);
  spi1_driver.disable();

  DW1000_Delay();
}
/*---------------------------------------------------------------------------*/
static void
spiRead(dwDevice_t* dev, const void *header, uint16_t headerLength,
                                 void* data, uint16_t dataLength)
{
  uint8_t *p_header = (uint8_t *)header;
  uint8_t *p_data = (uint8_t *)data;
  spi1_driver.enable();
  nrf_gpio_pin_clear(SPI_DWT_CS);

  spi1_driver.transfer(p_header, headerLength, NULL, 0);
  spi1_driver.transfer(NULL, 0, p_data, dataLength);

#if DEBUG_MODULE > 1
  int i;
  PRINTF("spiRead header length %d\n", headerLength);
  for ( i = 0; i < headerLength; i++) {
    PRINTF("%d ", p_header[i]);
  }
  PRINTF("\nspiRead data length %d\n", dataLength);
  for ( i = 0; i < dataLength; i++) {
    PRINTF("%d ", p_data[i]);
  }
  PRINTF("\n");
#endif

  nrf_gpio_pin_set(SPI_DWT_CS);
  spi1_driver.disable();
}
/*---------------------------------------------------------------------------*/
static void
spiWrite(dwDevice_t* dev, const void *header, uint16_t headerLength,
                                  const void* data, uint16_t dataLength)
{
  uint8_t *p_header = (uint8_t *)header;
  uint8_t *p_data = (uint8_t *)data;
  spi1_driver.enable();
  nrf_gpio_pin_clear(SPI_DWT_CS);

  spi1_driver.transfer(p_header, headerLength, NULL, 0);
  spi1_driver.transfer(p_data, dataLength, NULL, 0);

  #if DEBUG_MODULE > 1
    int i;
    PRINTF("spiWrite header length %d\n", headerLength);
    for ( i = 0; i < headerLength; i++) {
      PRINTF("%d ", p_header[i]);
    }
    PRINTF("\nspiWrite data length %d\n", dataLength);
    for ( i = 0; i < dataLength; i++) {
      PRINTF("%d ", p_data[i]);
    }
    PRINTF("\n");
  #endif

  nrf_gpio_pin_set(SPI_DWT_CS);
  spi1_driver.disable();
}
/*---------------------------------------------------------------------------*/
static void
spiSetSpeed(dwDevice_t* dev, dwSpiSpeed_t speed)
{

}
/*---------------------------------------------------------------------------*/
static void
delayms(dwDevice_t* dev, unsigned int delay)
{
  clock_delay(delay);
}
/*---------------------------------------------------------------------------*/
static void
reset(dwDevice_t *dev)
{
  nrf_gpio_cfg_output(DWT_RESET);
  nrf_gpio_pin_clear(DWT_RESET);
  nrf_gpio_cfg_input(DWT_RESET, NRF_GPIO_PIN_NOPULL);
  clock_delay(5);
}
/*---------------------------------------------------------------------------*/
// Set up opertions with mocks
static dwOps_t ops = {
  .spiRead = spiRead,
  .spiWrite = spiWrite,
  .spiSetSpeed = spiSetSpeed,
  .delayms = delayms,
  .reset = reset
};
static dwDevice_t dev;
/*---------------------------------------------------------------------------*/
static void
txcallback(void) {

}
/*---------------------------------------------------------------------------*/
static void
rxcallback(void) {

}
/*---------------------------------------------------------------------------*/
static void
dwt_init(void)
{
  nrf_gpio_cfg_output(SPI_DWT_CS);
  nrf_gpio_pin_set(SPI_DWT_CS);
  nrf_gpio_cfg_output(SPI_DWT_SCLK);
  nrf_gpio_cfg_output(SPI_DWT_MOSI);
  nrf_gpio_cfg_input(SPI_DWT_MISO, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_input(DWT_RESET, NRF_GPIO_PIN_NOPULL);

  int result;
  spi_master_config_t config;
	config.freq =  SPI_FREQUENCY_FREQUENCY_M8;
	config.sck_pin = SPI_DWT_SCLK;
	config.mosi_pin = SPI_DWT_MOSI;
	config.miso_pin = SPI_DWT_MISO;
	config.irq_priority = APP_IRQ_PRIORITY_LOW;
	config.order = SPI_MSB;
	config.mode = SPI_MODE0;
	spi1_driver.init(&config);
  nrf_delay_ms(100);

  // initialize dw1000 chip
  dwInit(&dev, &ops);
/*
 uint8_t readBuf[4] = {0};
 uint32_t *ptrBuf = (uint32_t*)readBuf;

 reset(NULL);
 nrf_delay_ms(200);
 reset(NULL);
 nrf_delay_ms(200);
 DW1000_readData(DW1000_DEV_ID, 0, readBuf, 4);
  PRINTF("device id = 0x%02X\r\n", *ptrBuf);


  PRINTF("readBuf[0]=0x%02x\n", readBuf[0]);
  PRINTF("readBuf[1]=0x%02x\n", readBuf[1]);
  PRINTF("readBuf[2]=0x%02x\n", readBuf[2]);
  PRINTF("readBuf[3]=0x%02x\n", readBuf[3]);
  while(1){};
  */
  // Configure the dw1000 chip
   result = dwConfigure(&dev);
  if (result == 0) {
    PRINTF("[OK]\r\n");
    dwEnableAllLeds(&dev);
  }
  else {
    PRINTF("[ERROR]: %s\n", dwStrError(result));
  }
}
/*---------------------------------------------------------------------------*/
static int
dwt_configure(uint32_t opcode, uint32_t value)
{
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
dwt_close( void)
{

	return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
dwt_open(void)
{
  dwTime_t delay = {.full = ANTENNA_DELAY/2};
  dwSetAntenaDelay(&dev, delay);

  dwAttachSentHandler(&dev, txcallback);
  dwAttachReceivedHandler(&dev, rxcallback);

  dwNewConfiguration(&dev);
  dwSetDefaults(&dev);
  dwEnableMode(&dev, MODE_SHORTDATA_FAST_ACCURACY);
  dwSetChannel(&dev, CHANNEL_2);
  dwSetPreambleCode(&dev, PREAMBLE_CODE_64MHZ_9);

  dwCommitConfiguration(&dev);
	return 0;
}
/*---------------------------------------------------------------------------*/
MODULE_CREATE(module_dwt, DWT_RADIO, dwt_init, dwt_open, dwt_close, dwt_configure);
