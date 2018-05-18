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
#include "iee-footpressure-arch.h"
#include <stdint.h>
#include <stdbool.h>
#include "dev/adc.h"
#include "dev/sensors.h"
#include "nrf_drv_saadc.h"
#include "frameworks/app_eventpool.h"
#include "frameworks/app_lifecycle.h"
#include "nrf_gpio.h"
#include "sys/clock.h"
#include "errno.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_init.h"
#if defined(USE_WDUI_STACK)
#include "wdui/wdui.h"
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
#define SAMPLES_IN_BUFFER 									1
#define ADC_REF_VOLTAGE_IN_MILLIVOLTS        1200
#define ADC_PRE_SCALING_COMPENSATION         3
#define DIODE_FWD_VOLT_DROP_MILLIVOLTS       270
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE)\
				((((ADC_VALUE) * ADC_REF_VOLTAGE_IN_MILLIVOLTS) / 1023) * ADC_PRE_SCALING_COMPENSATION)

static bool 	ieefp4_active = false;
static int16_t  inst_pressure_heel;
static int16_t  inst_pressure_outer_ball;
static int16_t  inst_pressure_inner_ball;
static int16_t  inst_pressure_thumb;
static uint32_t inst_timestamp;
static ieefp4_bsp_raw_data_func_t m_framework_raw_data_func;

extern TaskHandle_t g_contiki_thread;

/*---------------------------------------------------------------------------*/
/*static int
adc_channel_init_callback(uint32_t pin)
{
  int err_code;
  nrf_saadc_channel_config_t  config;

  //set configuration for saadc channel 1
  config.resistor_p = NRF_SAADC_RESISTOR_PULLDOWN;
  config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
  config.gain       = NRF_SAADC_GAIN1_6;
  config.reference  = NRF_SAADC_REFERENCE_VDD4;
  config.acq_time   = NRF_SAADC_ACQTIME_40US;
  config.mode       = NRF_SAADC_MODE_SINGLE_ENDED;
  config.pin_p      = (nrf_saadc_input_t)(nrf_drv_saadc_gpio_to_ain(pin));
  config.pin_n      = NRF_SAADC_INPUT_DISABLED;

  PRINTF("[iee footpressure] init channel %d\n", (nrf_saadc_input_t)(nrf_drv_saadc_gpio_to_ain(pin) - 1));
  err_code = nrf_drv_saadc_channel_init((nrf_saadc_input_t)(nrf_drv_saadc_gpio_to_ain(pin) - 1), &config);
  if (err_code == NRF_ERROR_INVALID_STATE) {
    return EINVALSTATE;
  } else if (err_code == NRF_ERROR_NO_MEM) {
    return ENOMEM;
  }
  return ENONE;
}*/
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
				inst_pressure_heel = ADC.sample(ADC_CHANNEL_HEEL);   // heel:L
				inst_pressure_inner_ball = ADC.sample(ADC_CHANNEL_INNER);   // inner:L  outter:R
				inst_pressure_outer_ball = ADC.sample(ADC_CHANNEL_OUTTER);   // thumb:L
				inst_pressure_thumb = ADC.sample(ADC_CHANNEL_THUMB);   // outter:L  inner:R
				inst_timestamp = clock_time();

				// PRINTF("[iee footpressure] thumb: %d hell: %d innor: %d outter: %d\n", inst_pressure_thumb, inst_pressure_heel,
				// 	inst_pressure_inner_ball, inst_pressure_outer_ball);
			}

			if (m_framework_raw_data_func != NULL && ieefp4_active) {
				m_framework_raw_data_func(inst_pressure_thumb, inst_pressure_outer_ball, inst_pressure_inner_ball, inst_pressure_heel, inst_timestamp);
			}
		} while (ieefp4_active);
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
int
ieefp4_bsp_init(void)
{
	if (!process_is_running(&ieefp4_bsp_process))
		process_start(&ieefp4_bsp_process, NULL);
	return ENONE;
}
/*---------------------------------------------------------------------------*/
int
ieefp4_bsp_poweron(void)
{
	if (ieefp4_active) {
		return EINVALSTATE;
	}
  PRINTF("[iee footpressure] power on\n");

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
ieefp4_bsp_poweroff(void)
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
int16_t
ieefp4_bsp_get_thumb(void)
{
	if (ieefp4_active) {
		return inst_pressure_thumb;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
int16_t
ieefp4_bsp_get_outer_ball(void)
{
	if (ieefp4_active) {
		return inst_pressure_outer_ball;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
int16_t
ieefp4_bsp_get_inner_ball(void)
{
	if (ieefp4_active) {
		return inst_pressure_inner_ball;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
int16_t
ieefp4_bsp_get_heel(void)
{
	if (ieefp4_active) {
		return inst_pressure_heel;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
int
hw_bsp_iee_footpressure4_open(void *args)
{

}
/*---------------------------------------------------------------------------*/
int
hw_bsp_iee_footpressure4_write(const void *buf, uint32_t len, uint32_t *offset)
{

}
/*---------------------------------------------------------------------------*/
int
hw_bsp_iee_footpressure4_read(void *buf, uint32_t len, uint32_t offset)
{

}
/*---------------------------------------------------------------------------*/
int
hw_bsp_iee_footpressure4_subscribe(void *buf, uint32_t len, HWCallbackHandler handler)
{

}
/*---------------------------------------------------------------------------*/
int
hw_bsp_iee_footpressure4_close(void *args)
{

}
/*---------------------------------------------------------------------------*/
