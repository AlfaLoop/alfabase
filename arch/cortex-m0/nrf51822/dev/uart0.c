/**
 *  Copyright (c) 2016 AlfaLoop, Inc. All Rights Reserved.
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
#include "uart0.h"
#include "bsp_init.h"

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_soc.h"
#include "nrf_uart.h"
//#include "dev/serial-line.h"

/* Scheduler includes. */
#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#endif
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
// extern SemaphoreHandle_t g_contiki_task_semephore;
extern TaskHandle_t g_contiki_thread;
static uart_config_t m_current_config;
static bool uart0_enable = false;
/*---------------------------------------------------------------------------*/
static void
simple_uart_put(uint8_t cr)
{
	NRF_UART0->TXD = (uint8_t) cr;
	while(NRF_UART0->EVENTS_TXDRDY != 1){
		// Wait for TXD data to be sent
    }
	NRF_UART0->EVENTS_TXDRDY = 0;
}
/*---------------------------------------------------------------------------*/
void
UART0_IRQHandler(void)
{
	BaseType_t yield_req = pdFALSE;
	uint8_t rx;

	if (nrf_uart_int_enable_check(NRF_UART0, NRF_UART_INT_MASK_ERROR) &&
        nrf_uart_event_check(NRF_UART0, NRF_UART_EVENT_ERROR))
  {
    nrf_uart_event_clear(NRF_UART0, NRF_UART_EVENT_ERROR);
    nrf_uart_int_disable(NRF_UART0, NRF_UART_INT_MASK_RXDRDY | NRF_UART_INT_MASK_ERROR);
  }
  else if (nrf_uart_int_enable_check(NRF_UART0, NRF_UART_INT_MASK_RXDRDY) &&
           nrf_uart_event_check(NRF_UART0, NRF_UART_EVENT_RXDRDY))
  {
    nrf_uart_event_clear(NRF_UART0, NRF_UART_EVENT_RXDRDY);
		if (m_current_config.cb != NULL) {
			 rx =(uint8_t) nrf_uart_rxd_get(NRF_UART0);
			 //PRINTF("rxd 0x%2x\n", rx);
			 m_current_config.cb (rx );
		}
		//serial_line_input_byte((uint8_t) nrf_uart_rxd_get(NRF_UART0));
#ifdef USE_FREERTOS
		vTaskNotifyGiveFromISR(g_contiki_thread, &yield_req);
#endif
  }

  if (nrf_uart_event_check(NRF_UART0, NRF_UART_EVENT_TXDRDY))
  {
		nrf_uart_event_clear(NRF_UART0, NRF_UART_EVENT_TXDRDY);
  }

  if (nrf_uart_event_check(NRF_UART0, NRF_UART_EVENT_RXTO))
  {
    nrf_uart_event_clear(NRF_UART0, NRF_UART_EVENT_RXTO);
    nrf_uart_task_trigger(NRF_UART0, NRF_UART_TASK_STARTRX);
  }
#ifdef USE_FREERTOS
  portYIELD_FROM_ISR(yield_req);
#endif
}
/*---------------------------------------------------------------------------*/

/** \brief Function to redirect the printf stream of stdio.h to UART
 * \param fd File descriptor, which is not used here
 * \param str Pointer to the string which needs to be sent by UART
 * \param len Length of the string sent
 * \return Returns length of the stream sent, as this will always be successful
 */
/*uint32_t
_write(int fd, char * str, int len){
	int i;
	for (i = 0; i < len; i++){
		simple_uart_put(str[i]);
	}
	return len;
}*/
/*---------------------------------------------------------------------------*/
static int
uart0_init(uart_config_t *config)
{
	nrf_gpio_cfg_output(config->tx);
	nrf_gpio_cfg_input(config->rx, NRF_GPIO_PIN_NOPULL);
	NRF_UART0->PSELTXD = config->tx;
	NRF_UART0->PSELRXD = config->rx;

	if(config->hwfc){	/* Enable hardware flow control */
		nrf_gpio_cfg_output(config->rts);
		nrf_gpio_cfg_input(config->cts, NRF_GPIO_PIN_NOPULL);
		NRF_UART0->PSELCTS = config->cts;
		NRF_UART0->PSELRTS = config->rts;
		NRF_UART0->CONFIG = (UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
	}

	NRF_UART0->BAUDRATE = (config->baudrate << UART_BAUDRATE_BAUDRATE_Pos);
	NRF_UART0->ENABLE = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
	NRF_UART0->TASKS_STARTTX = 1;
	NRF_UART0->TASKS_STARTRX = 1;
	NRF_UART0->EVENTS_RXDRDY = 0;

	// Enable UART interrupt
	NRF_UART0->INTENCLR = 0xffffffffUL;
	NRF_UART0->INTENSET = (UART_INTENSET_RXDRDY_Set << UART_INTENSET_RXDRDY_Pos);

	//nrf_uart_int_enable(NRF_UART0, NRF_UART_INT_MASK_RXDRDY | NRF_UART_INT_MASK_ERROR);
	// nrf_uart_int_enable(NRF_UART0, NRF_UART_INT_MASK_RXDRDY |NRF_UART_INT_MASK_tTO);
  /*  nrf_uart_int_disable(NRF_UART0, NRF_UART_INT_MASK_RXDRDY | NRF_UART_INT_MASK_ERROR);
	nrf_uart_event_clear(NRF_UART0, NRF_UART_EVENT_TXDRDY);
    nrf_uart_event_clear(NRF_UART0, NRF_UART_EVENT_RXTO);
    nrf_uart_int_enable(NRF_UART0, NRF_UART_INT_MASK_RXDRDY | NRF_UART_INT_MASK_ERROR);
	*/
	NVIC_SetPriority(UART0_IRQn, 3);
	NVIC_ClearPendingIRQ(UART0_IRQn);
	NVIC_EnableIRQ(UART0_IRQn);

	memcpy(&m_current_config, config, sizeof(uart_config_t));
	uart0_enable = true;
}
/*---------------------------------------------------------------------------*/
static int
uart0_disable(void)
{
	NRF_UART0->ENABLE = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);
	NRF_UART0->TASKS_STARTTX = 0;
	NRF_UART0->TASKS_STARTRX = 0;
	NRF_UART0->EVENTS_RXDRDY = 0;

	NRF_UART0->INTENCLR = 0xffffffffUL;
	NRF_UART0->INTENSET = (UART_INTENSET_RXDRDY_Disabled << UART_INTENSET_RXDRDY_Pos);

	memcpy(&m_current_config, 0x00, sizeof(uart_config_t));
	uart0_enable = false;
}
/*---------------------------------------------------------------------------*/
static int
uart0_tx_with_config(uart_config_t *config, uint8_t *data, uint32_t length)
{
	NRF_UART0->ENABLE = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);
	NRF_UART0->TASKS_STARTTX = 0;
	NRF_UART0->TASKS_STARTRX = 0;
	NRF_UART0->EVENTS_RXDRDY = 0;

	NRF_UART0->INTENCLR = 0xffffffffUL;
	NRF_UART0->INTENSET = (UART_INTENSET_RXDRDY_Disabled << UART_INTENSET_RXDRDY_Pos);

	nrf_gpio_cfg_output(config->tx);
	nrf_gpio_cfg_input(config->rx, NRF_GPIO_PIN_NOPULL);
	NRF_UART0->PSELTXD = config->tx;
	NRF_UART0->PSELRXD = config->rx;

	if(config->hwfc){	/* Enable hardware flow control */
		nrf_gpio_cfg_output(config->rts);
		nrf_gpio_cfg_input(config->cts, NRF_GPIO_PIN_NOPULL);
		NRF_UART0->PSELCTS = config->cts;
		NRF_UART0->PSELRTS = config->rts;
		NRF_UART0->CONFIG = (UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
	}

	NRF_UART0->BAUDRATE = (config->baudrate << UART_BAUDRATE_BAUDRATE_Pos);
	NRF_UART0->ENABLE = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
	NRF_UART0->TASKS_STARTTX = 1;
	NRF_UART0->TASKS_STARTRX = 1;
	NRF_UART0->EVENTS_RXDRDY = 0;

	// Enable UART interrupt
	NRF_UART0->INTENCLR = 0xffffffffUL;
	NRF_UART0->INTENSET = (UART_INTENSET_RXDRDY_Set << UART_INTENSET_RXDRDY_Pos);

	//nrf_uart_int_enable(NRF_UART0, NRF_UART_INT_MASK_RXDRDY | NRF_UART_INT_MASK_ERROR);
	// nrf_uart_int_enable(NRF_UART0, NRF_UART_INT_MASK_RXDRDY |NRF_UART_INT_MASK_tTO);
  /*  nrf_uart_int_disable(NRF_UART0, NRF_UART_INT_MASK_RXDRDY | NRF_UART_INT_MASK_ERROR);
	nrf_uart_event_clear(NRF_UART0, NRF_UART_EVENT_TXDRDY);
    nrf_uart_event_clear(NRF_UART0, NRF_UART_EVENT_RXTO);
    nrf_uart_int_enable(NRF_UART0, NRF_UART_INT_MASK_RXDRDY | NRF_UART_INT_MASK_ERROR);
	*/
	NVIC_SetPriority(UART0_IRQn, 3);
	NVIC_ClearPendingIRQ(UART0_IRQn);
	NVIC_EnableIRQ(UART0_IRQn);

	uint_fast8_t i = 0;
	for (i = 0; i < length; i ++) {
		simple_uart_put(data[i]);
	}

	if (uart0_enable){
		uart0_init(&m_current_config);
	}
	else
	{
		NRF_UART0->ENABLE = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);
		NRF_UART0->TASKS_STARTTX = 0;
		NRF_UART0->TASKS_STARTRX = 0;
		NRF_UART0->EVENTS_RXDRDY = 0;

		NRF_UART0->INTENCLR = 0xffffffffUL;
		NRF_UART0->INTENSET = (UART_INTENSET_RXDRDY_Disabled << UART_INTENSET_RXDRDY_Pos);
	}
}
/*---------------------------------------------------------------------------*/
static int
uart0_tx(uint8_t *data, uint32_t length)
{
	uint_fast8_t i = 0;
	for (i = 0; i < length; i ++) {
		simple_uart_put(data[i]);
	}
}
/*---------------------------------------------------------------------------*/
const struct uart_driver uart0 = {
	"uart0 driver",
	uart0_init,
	uart0_disable,
	uart0_tx_with_config,
	uart0_tx,
};
/*---------------------------------------------------------------------------*/;
