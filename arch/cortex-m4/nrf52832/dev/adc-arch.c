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
#include "adc-arch.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_error.h"
#include "errno.h"
#include "nrf_drv_saadc.h"
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
static uint8_t m_mode;
static adc_channel_config_get_t m_cb = NULL;
/*---------------------------------------------------------------------------*/
static void
saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
}
/*---------------------------------------------------------------------------*/
int
adc_arch_init(adc_config_t *config)
{
  int err_code;

  m_mode = config->mode;
  m_cb = config->cb;

  err_code = nrf_drv_saadc_init(NULL, saadc_callback);
  if (err_code != NRF_SUCCESS) {
    PRINTF("[adc-arch] nrf_drv_saadc_init error %d\n", err_code);
    return EINVALSTATE;
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
adc_arch_channel_init(uint32_t channel, void *config)
{
  int err_code;
  if (config == NULL) {
    return ENULLP;
  }

  if (channel > NRF_SAADC_CHANNEL_COUNT) {
    return EINVAL;
  }

  // uint8_t channel = (nrf_drv_saadc_gpio_to_ain(pin) - 1);
  nrf_saadc_channel_config_t  *channel_config = (nrf_saadc_channel_config_t  *)config;

  err_code = nrf_drv_saadc_channel_init(channel, channel_config);
	if (err_code != NRF_SUCCESS) {
		PRINTF("[main] nrf_drv_saadc_channel_init error %d\n", err_code);
    return EINTERNAL;
	}
	return ENONE;
}
/*---------------------------------------------------------------------------*/
int16_t
adc_arch_sample(uint32_t channel)
{
  int err_code;
  nrf_saadc_value_t adc_value;

  if (channel > NRF_SAADC_CHANNEL_COUNT) {
    return EINVAL;
  }
  // uint8_t channel = (nrf_drv_saadc_gpio_to_ain(pin) - 1);
  // Sample SAADC on two channels
  err_code = nrf_drv_saadc_sample_convert(channel, &adc_value);
  if (err_code != NRF_SUCCESS) {
    PRINTF("[adc-arch] nrf_drv_saadc_sample_convert error %d\n", err_code);
    return 0;
  }
  return adc_value;
}
/*---------------------------------------------------------------------------*/
int
adc_arch_channel_uninit(uint32_t channel)
{
  int err_code;
  if (channel > NRF_SAADC_CHANNEL_COUNT) {
    return EINVAL;
  }
	err_code = nrf_drv_saadc_channel_uninit(channel);
  if (err_code != NRF_SUCCESS) {
    return EBUSY;
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
const struct adc_driver nrf_adc_arch_driver = {
	.name = "adc_arch",
	.init = adc_arch_init,
	.channel_init = adc_arch_channel_init,
	.sample = adc_arch_sample,
	.channel_uninit = adc_arch_channel_uninit
};
/*---------------------------------------------------------------------------*/
