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
#include "contiki.h"
#include "spi0.h"
#include <stdint.h>
#include <stdbool.h>
#include "nrf_drv_common.h"
#include "nrf_gpio.h"
#include "nrf_spi.h"
#include "nrf_assert.h"
#include "app_util_platform.h"
#include "errno.h"
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

//#define SPI_INSTANCE  0 /**< SPI instance index. */
//static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
static bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */
static bool transfer_in_progress ;
static spi_master_config_t m_config;
/*---------------------------------------------------------------------------*/
/*static void
spi_event_handler(nrf_drv_spi_evt_t const * p_event)
{
    spi_xfer_done = true;
	//PRINTF("spi_xfer_done\n");
}*/
/*---------------------------------------------------------------------------*/
static int
spi0_init(spi_master_config_t *config)
{
	uint32_t errcode;

	// Copy parameters
	m_config.sck_pin = config->sck_pin;
	m_config.mosi_pin = config->mosi_pin;
	m_config.miso_pin = config->miso_pin;
	m_config.freq = config->freq;
	m_config.mode = config->mode;
	m_config.order = config->order;
	m_config.irq_priority = config->irq_priority;
	m_config.cs_pin = config->cs_pin;

	// Configure pins used by the peripheral:
	if (m_config.mode <= SPI_MODE1)
    {
        nrf_gpio_pin_clear(m_config.sck_pin);
    }
    else
    {
        nrf_gpio_pin_set(m_config.sck_pin);
    }

	NRF_GPIO->PIN_CNF[m_config.sck_pin] =
	(GPIO_PIN_CNF_DIR_Output        << GPIO_PIN_CNF_DIR_Pos)
  | (GPIO_PIN_CNF_INPUT_Connect     << GPIO_PIN_CNF_INPUT_Pos)
  | (GPIO_PIN_CNF_PULL_Disabled     << GPIO_PIN_CNF_PULL_Pos)
  | (GPIO_PIN_CNF_DRIVE_S0S1        << GPIO_PIN_CNF_DRIVE_Pos)
  | (GPIO_PIN_CNF_SENSE_Disabled    << GPIO_PIN_CNF_SENSE_Pos);


	nrf_gpio_pin_clear(m_config.mosi_pin);
  nrf_gpio_cfg_output(m_config.mosi_pin);

	nrf_gpio_cfg_input(m_config.miso_pin, NRF_GPIO_PIN_NOPULL);

	NRF_SPI_Type * p_spi = (NRF_SPI_Type *) NRF_SPI0;
	nrf_spi_pins_set(p_spi, m_config.sck_pin, m_config.mosi_pin, m_config.miso_pin);
	nrf_spi_frequency_set(p_spi,
            (nrf_spi_frequency_t)m_config.freq);
	nrf_spi_configure(p_spi,
            (nrf_spi_mode_t)m_config.mode,
            (nrf_spi_bit_order_t)m_config.order);

	// nrf_spi_enable(p_spi);
	transfer_in_progress = false;
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
spi0_enable(void)
{
	NRF_SPI_Type * p_spi = (NRF_SPI_Type *) NRF_SPI0;
	nrf_spi_enable(p_spi);
	nrf_gpio_pin_clear(m_config.cs_pin);
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
spi0_disable(void)
{
	NRF_SPI_Type * p_spi = (NRF_SPI_Type *) NRF_SPI0;
	nrf_spi_disable(p_spi);
	nrf_gpio_pin_set(m_config.cs_pin);
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
spi0_transfer(uint8_t * const p_tx_buf, const uint16_t tx_buf_len,
			  uint8_t * const p_rx_buf, const uint16_t rx_buf_len)
{
	uint32_t errcode;
	uint32_t bytes_transferred = 0;
	uint32_t total_bytes = (tx_buf_len >= rx_buf_len) ? tx_buf_len : rx_buf_len;
	uint32_t tx_used = tx_buf_len;
	uint32_t rx_used = rx_buf_len;
	uint8_t rx_data;
	spi_xfer_done = false;

	NRF_SPI_Type *p_spi = (NRF_SPI_Type *) NRF_SPI0;
	nrf_spi_event_clear(p_spi, NRF_SPI_EVENT_READY);

	while (total_bytes > 0)
	{
		if (tx_used > 0)
		{
			nrf_spi_txd_set(p_spi, p_tx_buf[bytes_transferred]);
			tx_used--;
		}
		else if ( rx_used > 0)
		{
			nrf_spi_txd_set(p_spi, SPI_DEFAULT_TX_BYTE);
		}

		while (!nrf_spi_event_check(p_spi, NRF_SPI_EVENT_READY)) {}
		nrf_spi_event_clear(p_spi, NRF_SPI_EVENT_READY);

		rx_data = nrf_spi_rxd_get(p_spi);
		if (rx_used > 0)
		{
			p_rx_buf[bytes_transferred] = rx_data;
			rx_used--;
		}

		bytes_transferred++;
		total_bytes--;
	}

	return ENONE;
}
/*---------------------------------------------------------------------------*/
const struct spi_driver spi0_driver = {
	"spi0 driver",
	spi0_init,
	spi0_enable,
	spi0_disable,
	spi0_transfer
};
/*---------------------------------------------------------------------------*/
