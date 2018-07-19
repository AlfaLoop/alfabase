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
#include "bsp_e32ttl.h"
#include <stdint.h>
#include <stdbool.h>
#include "frameworks/hw/hw_api.h"
#include "frameworks/hw/hw_api_null.h"
#include "frameworks/app_eventpool.h"
#include "frameworks/app_lifecycle.h"
#include "nrf_gpio.h"
#include "sys/clock.h"
#include "dev/uart.h"
#include "nrf_delay.h"
#include "errno.h"
#include "bsp_init.h"
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
#define LORA_TX_PIN     15u
#define LORA_RX_PIN     16u
#define LORA_M0_PIN     18u
#define LORA_M1_PIN     17u
#define LORA_AUX_PIN    14u
/*---------------------------------------------------------------------------*/
#define E32_NORMAL_MODE		0x00
#define E32_WAKEUP_MODE		0x01
#define E32_WOR_MODE		  0x02
#define E32_SLEEP_MODE		0x03
/*---------------------------------------------------------------------------*/
enum {
	E32TTL_SERIAL_8N1 = 0x00,
	E32TTL_SERIAL_8O1 = 0x01,
	E32TTL_SERIAL_8E1 = 0x02
};
/*---------------------------------------------------------------------------*/
enum {
	E32TTL_BAUDRATE_1200 = 0x00,
	E32TTL_BAUDRATE_2400 = 0x01,
	E32TTL_BAUDRATE_4800 = 0x02,
	E32TTL_BAUDRATE_9600 = 0x03,
	E32TTL_BAUDRATE_19200 = 0x04,
	E32TTL_BAUDRATE_38400 = 0x05,
	E32TTL_BAUDRATE_57600 = 0x06,
	E32TTL_BAUDRATE_115200 = 0x07
};
/*---------------------------------------------------------------------------*/
enum {
	E32TTL_BPS_0_3K = 0x00,
	E32TTL_BPS_1_2K = 0x01,
	E32TTL_BPS_2_4K = 0x02,
	E32TTL_BPS_4_8K = 0x03,
	E32TTL_BPS_9_6K = 0x04,
	E32TTL_BPS_19_2K = 0x05
};
/*---------------------------------------------------------------------------*/
enum {
	E32TTL_DYNAMIC_MODE = 0,
	E32TTL_STATIC_MODE = 1
};
/*---------------------------------------------------------------------------*/
enum {
	E32TTL_IO_OPEN_DRAIN = 0,
	E32TTL_IO_PUSH_PULL = 1,
};
/*---------------------------------------------------------------------------*/
enum {
	E32TTL_WOR_250MS = 0,
	E32TTL_WOR_500MS = 1,
	E32TTL_WOR_750MS = 2,
	E32TTL_WOR_1000MS = 3,
	E32TTL_WOR_1250MS = 4,
	E32TTL_WOR_1500MS = 5,
	E32TTL_WOR_1750MS = 6,
	E32TTL_WOR_2000MS = 7
};
/*---------------------------------------------------------------------------*/
enum {
	E32TTL_FEC_DISABLE = 0,
	E32TTL_FEC_ENABLE = 1,
};
/*---------------------------------------------------------------------------*/
enum {
	E32TTL_POWER_20DBM = 0,
	E32TTL_POWER_17DBM = 1,
	E32TTL_POWER_14DBM = 2,
	E32TTL_POWER_10DBM = 3,
};
/*---------------------------------------------------------------------------*/
__packed struct e32ttl_setup_packets {
	uint8_t head;
	uint8_t addr_h;
	uint8_t addr_l;
	uint8_t speed;
	uint8_t channel;
	uint8_t option;
};
/*---------------------------------------------------------------------------*/
__packed struct e32ttl_sped_packets {
	uint8_t serial : 2;
	uint8_t baudrate : 3;
	uint8_t bps :3 ;
};
/*---------------------------------------------------------------------------*/
__packed struct e32ttl_option_packets {
	uint8_t power : 2 ;
	uint8_t fec : 1 ;
	uint8_t wor : 3 ;
	uint8_t io : 1;
	uint8_t mode : 1;
};
/*---------------------------------------------------------------------------*/
static uint8_t reset_cmd[3] = {0xC4, 0xC4, 0xC4};
static uint8_t version_cmd[3] = {0xC3, 0xC3, 0xC3};
static uint8_t get_params_cmd[3] = {0xC1, 0xC1, 0xC1};
static uint8_t set_params_cmd[6] = {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00};
static bool m_uart_active = false;
static HWCallbackHandler m_sensor_event_callback = NULL;
extern const struct uart_driver uart0;
static uint8_t default_address_h = 0x00;
static uint8_t default_address_l = 0x01;     // default
static uint8_t default_channel = 0x0D;       // default channel
/*---------------------------------------------------------------------------*/
static void
lora_uart_rx_handler(uint8_t data)
{
	PRINTF("[bsp e32ttl] rx %02X\n", data);
}
/*---------------------------------------------------------------------------*/
static uart_config_t lora_uart_cfg = {
	.tx = LORA_TX_PIN,
	.rx = LORA_RX_PIN,
	.cts = 0,
	.rts = 0,
	.baudrate = UART_BAUDRATE_BAUDRATE_Baud9600,
	.hwfc = false,
	.cb = lora_uart_rx_handler
};
/*---------------------------------------------------------------------------*/
static uint8_t
get_option_packets(struct e32ttl_option_packets *p_option)
{
	static uint8_t byte;
	byte = p_option->mode << 7;
	byte |= p_option->io << 6;
	byte |= p_option->wor << 3;
	byte |= p_option->fec << 2;
	byte |= p_option->power;
	return byte;
}
/*---------------------------------------------------------------------------*/
static uint8_t
get_sped_packets(struct e32ttl_sped_packets *p_sped)
{
	static uint8_t byte;
	byte = p_sped->serial << 5;
	byte |= p_sped->baudrate << 3;
	byte |= p_sped->bps ;
	return byte;
}
/*---------------------------------------------------------------------------*/
static void
e32ttl_cfg_normal_mode(void)
{
	nrf_gpio_pin_clear(LORA_M0_PIN);
	nrf_gpio_pin_clear(LORA_M1_PIN);
	nrf_delay_ms(5);
}
/*---------------------------------------------------------------------------*/
static void
e32ttl_cfg_wakeup_mode(void)
{
	nrf_gpio_pin_set(LORA_M0_PIN);
	nrf_gpio_pin_clear(LORA_M1_PIN);
	nrf_delay_ms(5);
}
/*---------------------------------------------------------------------------*/
static void
e32ttl_cfg_wake_on_radio_mode(void)
{
	nrf_gpio_pin_clear(LORA_M0_PIN);
	nrf_gpio_pin_set(LORA_M1_PIN);
	nrf_delay_ms(5);
}
/*---------------------------------------------------------------------------*/
static void
e32ttl_cfg_sleep_mode(void)
{
	nrf_gpio_pin_set(LORA_M0_PIN);
	nrf_gpio_pin_set(LORA_M1_PIN);
	nrf_delay_ms(5);
}
/*---------------------------------------------------------------------------*/
static void
e32ttl_send_data(uint16_t address, uint8_t *data, uint16_t length)
{
	uart0.tx(data, length);
	nrf_delay_ms(25);
}
/*---------------------------------------------------------------------------*/
int
bsp_e32ttl_open(void *args)
{
  if (m_uart_active) {
    return EINVALSTATE;
  }
	uart0.init(&lora_uart_cfg);
	m_uart_active = true;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_e32ttl_write(const void *buf, uint32_t len, uint32_t offset)
{
	switch (offset) {
		case 0:
		{
		}
		break;
		case 1:
		{
		}
		break;
		case 2:
		{
		}
		break;
	}
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_e32ttl_read(void *buf, uint32_t len, uint32_t offset)
{
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_e32ttl_subscribe(void *buf, uint32_t len, HWCallbackHandler handler)
{
  m_sensor_event_callback = handler;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_e32ttl_close(void *args)
{
	if (!m_uart_active) {
    return EINVALSTATE;
  }
	uart0.disable();
	m_uart_active = false;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static void
app_terminating(void)
{
	bsp_e32ttl_close(NULL);
	m_uart_active = false;
}
/*---------------------------------------------------------------------------*/
static struct app_lifecycle_event lifecycle_event = {
	.name = "hw_bsp_e32ttl",
	.terminating = app_terminating
};
/*---------------------------------------------------------------------------*/
int
bsp_e32ttl_init(void)
{
	app_lifecycle_register(&lifecycle_event);

	// default is in sleep mode: for configuration
	e32ttl_cfg_sleep_mode();

		// initialize default params
	struct e32ttl_sped_packets sped_packets;
	sped_packets.bps = E32TTL_BPS_19_2K;		// 5 : 2
	sped_packets.baudrate = E32TTL_BAUDRATE_9600;		// 3
	sped_packets.serial = E32TTL_SERIAL_8N1;		// 0

	// wake on radio
	struct e32ttl_option_packets option_packets;
	option_packets.mode = E32TTL_STATIC_MODE;
	option_packets.io = E32TTL_IO_PUSH_PULL;
	option_packets.wor = E32TTL_WOR_2000MS;
	option_packets.fec = E32TTL_FEC_ENABLE;
	option_packets.power = E32TTL_POWER_20DBM;

	struct e32ttl_setup_packets packets;
	packets.head = 0xC0;
	packets.addr_h = default_address_h;
	packets.addr_l = default_address_l;
	packets.speed = get_sped_packets(&sped_packets);
	packets.channel = default_channel;
	packets.option = get_option_packets(&option_packets);

	memcpy(set_params_cmd, &packets, 6);

	PRINTF("[bsp e32ttl] set params: ");
	for (int i = 0; i < 6; i++)
		PRINTF("%02x ", set_params_cmd[i]);
	PRINTF("\n");

	uart0.tx(set_params_cmd, 6);
	nrf_delay_ms(10);

	m_uart_active = false;
}
/*---------------------------------------------------------------------------*/
