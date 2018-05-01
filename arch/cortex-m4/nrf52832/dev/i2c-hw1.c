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
#include "i2c-hw0.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_error.h"
#include "errno.h"
#include "app_util_platform.h"
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

static const nrf_drv_twi_t m_twi_instance = NRF_DRV_TWI_INSTANCE(1);
static bool twi_error = false;
/*---------------------------------------------------------------------------*/
static int
i2chw1_init(const i2c_config_t *config)
{
	uint32_t err_code = ENONE;
	const nrf_drv_twi_config_t twi_config = {
			 // For mpu9250's pin set.
			.scl                = config->scl,
			.sda                = config->sda,
			.frequency          = NRF_TWI_FREQ_400K,
			.interrupt_priority = APP_IRQ_PRIORITY_LOW
	 };
  nrf_drv_twi_init(&m_twi_instance, &twi_config, NULL, NULL);
	return err_code;
}
/*---------------------------------------------------------------------------*/
static int
i2chw1_enable(void)
{
	nrf_drv_twi_enable(&m_twi_instance);
}
/*---------------------------------------------------------------------------*/
static int
i2chw1_disable(void)
{
	nrf_drv_twi_disable(&m_twi_instance);
}
/*---------------------------------------------------------------------------*/
static int
i2chw1_transfer(uint8_t type, uint8_t address, uint32_t length, uint8_t * p_data, uint8_t pending)
{
	uint32_t err_code = ENONE;
	if (type == I2C_TX) {
		if (pending) {
			twi_error = false;
			nrf_drv_twi_tx(&m_twi_instance, address >> 1, p_data, length, false);
			if(twi_error) {
				PRINTF("[i2chw0] tx pedding error\n");
			}
		}
		else {
			twi_error = false;
			nrf_drv_twi_tx(&m_twi_instance, address >> 1, p_data, length, true);
			if(twi_error) {
				PRINTF("[i2chw0] tx error\n");
			}
		}
	}
	else if (type == I2C_RX) {
    twi_error = false;
    nrf_drv_twi_rx(&m_twi_instance, address >> 1, p_data, length);
    if(twi_error) return false;
	}
	return err_code;
}
/*---------------------------------------------------------------------------*/
const struct i2c_driver nrf_twi_hw_driver1 = {
	"i2chw1",
	i2chw1_init,
	i2chw1_enable,
	i2chw1_disable,
	i2chw1_transfer
};
/*---------------------------------------------------------------------------*/
