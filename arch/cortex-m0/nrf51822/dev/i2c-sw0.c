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
#include "i2c-sw0.h"
#include "twi_master0.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_error.h"
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

/*---------------------------------------------------------------------------*/
int
i2csw0_init(const i2c_config_t *config)
{
	uint32_t err_code = ENONE;

	bool statu = twi_master0_init(config->sda, config->scl);
    PRINTF("[i2csw0] init sda %d scl %d\n", config->sda, config->scl);
	if (!statu) {
		PRINTF("[i2csw0] I2C sw bus is stuck.");
	}
	return err_code;
}
/*---------------------------------------------------------------------------*/
int
i2csw0_enable(void)
{
}
/*---------------------------------------------------------------------------*/
int
i2csw0_disable(void)
{
}
/*---------------------------------------------------------------------------*/
int
i2csw0_transfer(uint8_t type, uint8_t address, uint32_t length, uint8_t * p_data, uint8_t pending)
{
	uint32_t err_code;
	if (type == I2C_TX) {
		if (pending) {
			if(!twi_master0_transfer(address, p_data, length, false)) {
				PRINTF("[i2csw0] twi0_master_transfer pedding error %d\n", err_code);
			}
		}
		else {
			if(!twi_master0_transfer(address, p_data, length, true)) {
				PRINTF("[i2csw0] twi0_master_transfer  error %d\n", err_code);
			}
		}
	}
	else if (type == I2C_RX) {
		if (pending) {
			if(!twi_master0_transfer(address | TWI0_READ_BIT, p_data, length, false)) {
				PRINTF("[i2csw0] twi0_master_transfer pedding error %d\n", err_code);
			}
		}
		else {
			if(!twi_master0_transfer(address | TWI0_READ_BIT, p_data, length, true)) {
				PRINTF("[i2csw0] twi0_master_transfer  error %d\n", err_code);
			}
		}

	}

	if (err_code != NRF_SUCCESS) {
		PRINTF("[i2csw0] i2c_operate error %d\n", err_code);
	}
	return err_code;
}
/*---------------------------------------------------------------------------*/
const struct i2c_driver nrf_twi_sw_driver0 = {
	"i2csw0",
	i2csw0_init,
	i2csw0_enable,
	i2csw0_disable,
	i2csw0_transfer
};
/*---------------------------------------------------------------------------*/
