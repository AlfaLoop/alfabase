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
#include "bsp_ieefp4.h"
#include <stdint.h>
#include <stdbool.h>
#include "dev/adc.h"
#include "nrf_drv_saadc.h"
#include "frameworks/app_eventpool.h"
#include "frameworks/app_lifecycle.h"
#include "nrf_gpio.h"
#include "sys/clock.h"
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
#define SAMPLES_IN_BUFFER 									1
#define ADC_REF_VOLTAGE_IN_MILLIVOLTS        1200
#define ADC_PRE_SCALING_COMPENSATION         3
#define DIODE_FWD_VOLT_DROP_MILLIVOLTS       270
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE)\
				((((ADC_VALUE) * ADC_REF_VOLTAGE_IN_MILLIVOLTS) / 1023) * ADC_PRE_SCALING_COMPENSATION)

static bool 	ieefp4_active = false;

static ieefp4_data_t ieefp4_data_inst;

/*---------------------------------------------------------------------------*/
PROCESS(ieefp4_bsp_process, "iee_pressure watch");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ieefp4_bsp_process, ev, data)
{
	static struct etimer st;
	PROCESS_BEGIN();
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_POLL ) );

		do {
			etimer_set(&st, CLOCK_SECOND / 10);
			PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&st));

			if (ieefp4_active) {
				ieefp4_data_inst.heel = ADC.sample(ADC_CHANNEL_HEEL);
				ieefp4_data_inst.inner_ball = ADC.sample(ADC_CHANNEL_INNER);
				ieefp4_data_inst.outer_ball = ADC.sample(ADC_CHANNEL_OUTTER);
				ieefp4_data_inst.thumb = ADC.sample(ADC_CHANNEL_THUMB);
				ieefp4_data_inst.timestamp = clock_time();
				// PRINTF("[bsp ieefp4] thumb: %d hell: %d innor: %d outter: %d\n", ieefp4_data_inst.thumb, ieefp4_data_inst.heel,
				// 	ieefp4_data_inst.inner_ball, ieefp4_data_inst.outer_ball);
			}

			// if (m_framework_raw_data_func != NULL && ieefp4_active) {
			// 	m_framework_raw_data_func(ieefp4_data_inst.thumb, ieefp4_data_inst.outer_ball, ieefp4_data_inst.inner_ball, ieefp4_data_inst.heel, inst_timestamp);
			// }
		} while (ieefp4_active);
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
bool
ieefp4_bsp_activated(void)
{
	return ieefp4_active;
}
/*---------------------------------------------------------------------------*/
uint8_t
ieefp4_bsp_get_type(void)
{
#if BOARD_DEV_VERSION == BOARD_FOOT_V2_RIGHT || BOARD_DEV_VERSION == BOARD_FOOT_V3_RIGHT
	return 0;
#elif BOARD_DEV_VERSION == BOARD_FOOT_V2_LEFT || BOARD_DEV_VERSION == BOARD_FOOT_V3_LEFT
	return 1;
#endif
}
/*---------------------------------------------------------------------------*/
int
bsp_ieefp4_open(void *args)
{
	if (ieefp4_active) {
		return EINVALSTATE;
	}
  PRINTF("[bsp ieefp4] power on\n");

	nrf_saadc_channel_config_t  heel_config;
  heel_config.resistor_p = NRF_SAADC_RESISTOR_PULLDOWN;
  heel_config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
  heel_config.gain       = NRF_SAADC_GAIN1_6;
  heel_config.reference  = NRF_SAADC_REFERENCE_VDD4;
  heel_config.acq_time   = NRF_SAADC_ACQTIME_40US;
  heel_config.mode       = NRF_SAADC_MODE_SINGLE_ENDED;
  heel_config.pin_p      = (nrf_saadc_input_t)(nrf_drv_saadc_gpio_to_ain(ADC_INPUT_PIN_HEEL));
  heel_config.pin_n      = NRF_SAADC_INPUT_DISABLED;

	nrf_saadc_channel_config_t  inner_config;
	inner_config.resistor_p = NRF_SAADC_RESISTOR_PULLDOWN;
	inner_config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
	inner_config.gain       = NRF_SAADC_GAIN1_6;
	inner_config.reference  = NRF_SAADC_REFERENCE_VDD4;
	inner_config.acq_time   = NRF_SAADC_ACQTIME_40US;
	inner_config.mode       = NRF_SAADC_MODE_SINGLE_ENDED;
	inner_config.pin_p      = (nrf_saadc_input_t)(nrf_drv_saadc_gpio_to_ain(ADC_INPUT_PIN_INNER));
	inner_config.pin_n      = NRF_SAADC_INPUT_DISABLED;

	nrf_saadc_channel_config_t  outter_config;
	outter_config.resistor_p = NRF_SAADC_RESISTOR_PULLDOWN;
	outter_config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
	outter_config.gain       = NRF_SAADC_GAIN1_6;
	outter_config.reference  = NRF_SAADC_REFERENCE_VDD4;
	outter_config.acq_time   = NRF_SAADC_ACQTIME_40US;
	outter_config.mode       = NRF_SAADC_MODE_SINGLE_ENDED;
	outter_config.pin_p      = (nrf_saadc_input_t)(nrf_drv_saadc_gpio_to_ain(ADC_INPUT_PIN_OUTTER));
	outter_config.pin_n      = NRF_SAADC_INPUT_DISABLED;

	nrf_saadc_channel_config_t  thumb_config;
	thumb_config.resistor_p = NRF_SAADC_RESISTOR_PULLDOWN;
	thumb_config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
	thumb_config.gain       = NRF_SAADC_GAIN1_6;
	thumb_config.reference  = NRF_SAADC_REFERENCE_VDD4;
	thumb_config.acq_time   = NRF_SAADC_ACQTIME_40US;
	thumb_config.mode       = NRF_SAADC_MODE_SINGLE_ENDED;
	thumb_config.pin_p      = (nrf_saadc_input_t)(nrf_drv_saadc_gpio_to_ain(ADC_INPUT_PIN_THUMB));
	thumb_config.pin_n      = NRF_SAADC_INPUT_DISABLED;

  ADC.channel_init(ADC_CHANNEL_HEEL, (void *)&heel_config);
	ADC.channel_init(ADC_CHANNEL_INNER, (void *)&inner_config);
	ADC.channel_init(ADC_CHANNEL_OUTTER, (void *)&outter_config);
	ADC.channel_init(ADC_CHANNEL_THUMB, (void *)&thumb_config);

	ieefp4_active = true;
	process_post(&ieefp4_bsp_process, PROCESS_EVENT_POLL, NULL);
	return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_ieefp4_write(const void *buf, uint32_t len, uint32_t *offset)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
bsp_ieefp4_read(void *buf, uint32_t len, uint32_t offset)
{
	ieefp4_data_t *p_user_data = (ieefp4_data_t*)buf;
	if (ieefp4_active) {
		p_user_data->heel = ieefp4_data_inst.heel;
		p_user_data->inner_ball = ieefp4_data_inst.inner_ball;
		p_user_data->outer_ball = ieefp4_data_inst.outer_ball;
		p_user_data->thumb = ieefp4_data_inst.thumb;
		p_user_data->timestamp = ieefp4_data_inst.timestamp;
	}
	return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_ieefp4_subscribe(void *buf, uint32_t len, HWCallbackHandler handler)
{
	return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
bsp_ieefp4_close(void *args)
{
	if (!ieefp4_active)
		return EINVALSTATE;

	ADC.channel_uninit(ADC_CHANNEL_HEEL);
	ADC.channel_uninit(ADC_CHANNEL_INNER);
	ADC.channel_uninit(ADC_CHANNEL_OUTTER);
	ADC.channel_uninit(ADC_CHANNEL_THUMB);
	ieefp4_active = false;

	return ENONE;
}
/*---------------------------------------------------------------------------*/
static void
app_terminating(void)
{
	bsp_ieefp4_close(NULL);
}
/*---------------------------------------------------------------------------*/
static struct app_lifecycle_event lifecycle_event = {
	.name = "hw_bsp_ieefp4",
	.terminating = app_terminating
};
/*---------------------------------------------------------------------------*/
int
bsp_ieefp4_init(void)
{
	app_lifecycle_register(&lifecycle_event);

	if (!process_is_running(&ieefp4_bsp_process))
		process_start(&ieefp4_bsp_process, NULL);
	return ENONE;
}
/*---------------------------------------------------------------------------*/
