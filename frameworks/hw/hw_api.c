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
#include "hw_api.h"
#include "frameworks/app_lifecycle.h"
#include "frameworks/app_eventpool.h"
#include "frameworks/hw/null_hw.h"
#include "loader/symtab.h"
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
static hw_pin_mode_t *m_hw_pin_list = NULL;
static hw_api_bsp_terminating_callback m_terminating_callback = NULL;
static uint8_t m_total_pins;
/*---------------------------------------------------------------------------*/
Pin*
HWPin(void)
{
	static Pin null_instance;
	null_instance.setup = null_p_2_uint32_r_int;
	null_instance.output = null_p_2_uint32_r_int;
	null_instance.input = null_p_2_uint32_r_int;
	null_instance.read = null_p_1_uint32_r_int;
	null_instance.attachInterrupt = null_pin_watch_set;
	null_instance.detachInterrupt = null_pin_watch_close;
#if defined(USE_HARDWARE_PIN)
	if (bsp_hw_pin_api_retrieve() != NULL) {
		return bsp_hw_pin_api_retrieve();
	}
#endif
	return &null_instance;
}
/*---------------------------------------------------------------------------*/
static struct symbols symbolHWPin = {
	.name = "HWPin",
	.value = (void *)&HWPin
};
/*---------------------------------------------------------------------------*/
Uart*
HWUart(uint8_t number)
{
	static Uart null_instance;
	null_instance.open = null_uart_open;
	null_instance.close = null_uart_close;
	null_instance.send = null_uart_send;
#if defined(USE_HARDWARE_UART)
	if (bsp_hw_uart_api_retrieve(number) != NULL) {
		return bsp_hw_uart_api_retrieve(number);
	}
#endif
	return &null_instance;
}
/*---------------------------------------------------------------------------*/
static struct symbols symbolHWUart = {
	.name = "HWUart",
	.value = (void *)&HWUart
};
/*---------------------------------------------------------------------------*/
I2c*
HWI2c(uint8_t number)
{
	static I2c null_instance;
	null_instance.init = null_i2c_init;
	null_instance.write = null_i2c_write;
	null_instance.read = null_i2c_read;
	null_instance.close = null_i2c_close;
#if defined(USE_HARDWARE_I2C)
	if (bsp_hw_i2c_api_retrieve(number) != NULL) {
		return bsp_hw_uart_api_retrieve(number);
	}
#endif
	return &null_instance;
}
/*---------------------------------------------------------------------------*/
static struct symbols symbolHWI2c = {
	.name = "HWI2c",
	.value = (void *)&HWI2c
};
/*---------------------------------------------------------------------------*/
Spi*
HWSpi(uint8_t number)
{
	static Spi null_instance;
	null_instance.init = null_spi_init;
	null_instance.write = null_spi_write;
	null_instance.read = null_spi_read;
	null_instance.close = null_spi_close;
#if defined(USE_HARDWARE_SPI)
	if (bsp_hw_spi_api_retrieve(number) != NULL) {
		return bsp_hw_uart_api_retrieve(number);
	}
#endif
	return &null_instance;
}
/*---------------------------------------------------------------------------*/
static struct symbols symbolHWSpi = {
	.name = "HWSpi",
	.value = (void *)&HWSpi
};
/*---------------------------------------------------------------------------*/
static void
app_terminating(void)
{
	if (m_terminating_callback != NULL) {
		m_terminating_callback();
	}
}
/*---------------------------------------------------------------------------*/
static struct app_lifecycle_event lifecycle_event = {
	.name = "hw_api",
	.terminating = app_terminating
};
/*---------------------------------------------------------------------------*/
void
hw_api_init(void)
{
#if defined(USE_HARDWARE)
	app_lifecycle_register(&lifecycle_event);
#if defined(USE_HARDWARE_PIN)
	symtab_add(&symbolHWPin);
#endif /* USE_HARDWARE_PIN */
#if defined(USE_HARDWARE_UART)
	symtab_add(&symbolHWUart);
#endif /* USE_HARDWARE_UART */
#if defined(USE_HARDWARE_I2C)
	symtab_add(&symbolHWI2c);
#endif /* USE_HARDWARE_I2C */
#if defined(USE_HARDWARE_SPI)
	symtab_add(&symbolHWSpi);
#endif /* USE_HARDWARE_SPI */
#endif  /* USE_HARDWARE */
}
/*---------------------------------------------------------------------------*/
bool
hw_api_check_pin(uint32_t pin, uint16_t type)
{
	int i = 0;
  bool enabled = false;
	hw_pin_mode_t *pin_mode = NULL;

	// PRINTF("[hw api] check pin %d type %d \n", pin, type);
	if (m_hw_pin_list == NULL) {
		PRINTF("[hw api] pin list is null\n");
		return enabled;
	}

	for (int i = 0; i < m_total_pins; i++) {
		pin_mode = &m_hw_pin_list[i];
		// PRINTF("[hw api] check %d pin %d mode %d\n",i, pin_mode->pin, pin_mode->mode);
		if ( (pin_mode->pin == pin) && (pin_mode->mode & type)) {
			// PRINTF("[hw api] pin %d exist\n", i);
			enabled = true;
			break;
		}
	}

  return enabled;
}
/*---------------------------------------------------------------------------*/
void
hw_api_bsp_init(const hw_pin_mode_t *pins, uint8_t num, hw_api_bsp_terminating_callback callback)
{
	m_hw_pin_list = &pins[0];
	m_total_pins = num;
	m_terminating_callback = callback;
}
/*---------------------------------------------------------------------------*/
