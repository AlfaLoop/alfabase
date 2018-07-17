/**
 *  Copyright (c) 2016 AlfaLoop Technology Co., Ltd. All Rights Reserved.
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
#include "contiki.h"
#include "dev/power.h"
#include "nrf_drv_adc.h"
#include "nrf_gpio.h"
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
static battery_level_correction_func_t m_battery_level_correction = NULL;
static battery_level_adc_channel_cb_t m_battery_level_adc_channel_init = NULL;
/*---------------------------------------------------------------------------*/
// int
// power_status(power_status_t *status)
// {
//   uint32_t err_code;

  // initialized pressure sensor adc configuration
	// m_adc_channel_config.resistor_p = NRF_SAADC_RESISTOR_PULLDOWN;
  // m_adc_channel_config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
  // m_adc_channel_config.gain       = NRF_SAADC_GAIN1_3;
  // m_adc_channel_config.reference  = NRF_SAADC_REFERENCE_VDD4;
  // m_adc_channel_config.acq_time   = NRF_SAADC_ACQTIME_40US;
  // m_adc_channel_config.mode       = NRF_SAADC_MODE_SINGLE_ENDED;
  // m_adc_channel_config.pin_p      = (nrf_saadc_input_t)(nrf_drv_saadc_gpio_to_ain(ARCH_BATTERY_ADC_PIN));
  // m_adc_channel_config.pin_n      = NRF_SAADC_INPUT_DISABLED;
	//
  // err_code = nrf_drv_saadc_channel_init(ARCH_BATTERY_ADC_CHANNEL, &m_adc_channel_config);
  // if (err_code != NRF_SUCCESS) {
  //   PRINTF("[power] nrf_drv_saadc_channel_init error %d\n", err_code);
  // }

//   //Sample SAADC on two channels
//   err_code = nrf_drv_saadc_sample_convert(ARCH_BATTERY_ADC_CHANNEL, &m_adc_value);
//   if (err_code != NRF_SUCCESS) {
//     PRINTF("[power] nrf_drv_saadc_sample_convert error %d\n", err_code);
//   }
//
//   PRINTF("[power] adc: %d\n", m_adc_value);
//   m_adc_result_millivolts = ADC_RESULT_IN_MILLI_VOLTS(m_adc_value);
//   PRINTF("[power] millivolts: %d\n", m_adc_result_millivolts);
//
//   m_adc_result_percent = battery_level_in_percent(m_adc_result_millivolts);          // Transform the millivolts value into battery level percent.
//   PRINTF("[power] percent: %d\n", m_adc_result_percent);
//
// 	status->power_source = POWER_SOURCE_BATTERY;
// 	status->battery_percent = m_adc_result_percent;
//
// 	nrf_drv_saadc_channel_uninit((nrf_drv_saadc_gpio_to_ain(BATTERY_ADC_PIN) - 1));
//   return ENONE;
// }
/*---------------------------------------------------------------------------*/
uint8_t
power_arch_get_battery_level(void)
{
	uint32_t err_code;
	nrf_drv_adc_channel_t  adc_channel_config;
	// uint8_t adc_channel = (nrf_drv_adc_gpio_to_ain(BATTERY_ADC_PIN) - 1);
	uint8_t power_level;
	int16_t adc_value;

	adc_channel_config.config.config.resolution = NRF_ADC_CONFIG_RES_10BIT;
	adc_channel_config.config.config.input = NRF_ADC_CONFIG_SCALING_INPUT_FULL_SCALE;
	adc_channel_config.config.config.reference = NRF_ADC_CONFIG_REF_VBG;
	adc_channel_config.config.config.ain = nrf_drv_adc_gpio_to_ain(BATTERY_ADC_PIN);
	adc_channel_config.p_next = NULL;

	if (m_battery_level_adc_channel_init != NULL) {
		m_battery_level_adc_channel_init();
	}


  //Sample SAADC on two channels
  err_code = nrf_drv_adc_sample_convert(&adc_channel_config, &adc_value);
  if (err_code != NRF_SUCCESS) {
    PRINTF("[power] nrf_drv_saadc_sample_convert error %d\n", err_code);
  }

	if (m_battery_level_correction != NULL) {
		power_level = m_battery_level_correction(adc_value);
	}

	nrf_drv_adc_channel_disable(&adc_channel_config);
	PRINTF("[power] percent: %d\n", power_level);
	return power_level;
}
/*---------------------------------------------------------------------------*/
uint8_t
power_arch_get_charged_status(void)
{
	// TODO: check the charged status

	return POWER_SOURCE_BATTERY;
}
/*---------------------------------------------------------------------------*/
int
power_arch_init(power_config_t *config)
{
	uint32_t err_code;
	nrf_drv_adc_config_t   adc_config;

	if (config->battery_level_correction != NULL) {
		m_battery_level_correction = config->battery_level_correction;
	}

	if (config->channel_init != NULL) {
		m_battery_level_adc_channel_init = config->channel_init;
	}

	// initialized pressure sensor adc configuration
	adc_config.interrupt_priority = 3;
	err_code = nrf_drv_adc_init(&adc_config, NULL);
	if (err_code != NRF_SUCCESS) {
		PRINTF("[power] init error %d\n", err_code);
	}

	return ENONE;
}
/*---------------------------------------------------------------------------*/
