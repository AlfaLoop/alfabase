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
#ifndef SPI_H_
#define SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Define macros to use for checking SPI transmission status depending
   on if it is possible to wait for TX buffer ready. This is possible
   on for example MSP430 but not on AVR. */
#ifdef SPI_WAITFORTxREADY
#define SPI_WAITFORTx_BEFORE() SPI_WAITFORTxREADY()
#define SPI_WAITFORTx_AFTER()
#define SPI_WAITFORTx_ENDED() SPI_WAITFOREOTx()
#else /* SPI_WAITFORTxREADY */
#define SPI_WAITFORTx_BEFORE()
#define SPI_WAITFORTx_AFTER() SPI_WAITFOREOTx()
#define SPI_WAITFORTx_ENDED()
#endif /* SPI_WAITFORTxREADY */

#define SPI_MODE0  0
#define SPI_MODE1  1
#define SPI_MODE2  2
#define SPI_MODE3  3

#define SPI_LSB	   1
#define SPI_MSB    0

#define SPI_FREQUENCY_K125 (0x02000000UL) /*!< 125kbps. */
#define SPI_FREQUENCY_K250 (0x04000000UL) /*!< 250kbps. */
#define SPI_FREQUENCY_K500 (0x08000000UL) /*!< 500kbps. */
#define SPI_FREQUENCY_M1 (0x10000000UL) /*!< 1Mbps. */
#define SPI_FREQUENCY_M2 (0x20000000UL) /*!< 2Mbps. */
#define SPI_FREQUENCY_M4 (0x40000000UL) /*!< 4Mbps. */
#define SPI_FREQUENCY_M8 (0x80000000UL) /*!< 8Mbps. */

#define SPI_PIN_DISCONNECTED 0xFFFFFFFF /**< A value used to the PIN deinitialization. */
#define SPI_DEFAULT_TX_BYTE  0x00       /**< Default byte (used to clock transmission
                                             from slave to the master) */

typedef struct
{
  uint32_t freq;          /**< SPI master frequency */
  uint32_t sck_pin;       /**< SCK pin number. */
  uint32_t miso_pin;      /**< MISO pin number. */
  uint32_t mosi_pin;      /**< MOSI pin number .*/
  uint32_t irq_priority;   /**< SPI master interrupt priority. */
  uint8_t order;    /**< Serial clock polarity ACTIVEHIGH or ACTIVELOW. */
  uint8_t mode;    /**< Serial clock phase LEADING or TRAILING. */
  uint32_t cs_pin;    /**< Serial clock phase LEADING or TRAILING. */
} spi_master_config_t;


struct spi_driver {
	char *name;

	/** Initialize the SPI driver */
	int (* init)(spi_master_config_t *config);

	/** Enable the SPI driver */
	int (* enable)(void);

	/** Disable the SPI driver */
	int (* disable)(void);

	/** Data transfer */
	int (* transfer)(uint8_t * const p_tx_buf, const uint16_t tx_buf_len,
					uint8_t * const p_rx_buf, const uint16_t rx_buf_len);
};



#ifdef __cplusplus
}
#endif
#endif /* SPI_H_ */
