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
#define ARCH_BATTERY_ADC_SOURCE                 ARCH_BATTERY_ADC_SOURCE_TYPE_CONF
#define ARCH_BATTERY_ADC_PIN                    ARCH_BATTERY_ADC_SOURCE_PIN_CONF
#define ARCH_BATTERY_ADC_VENDOR                 ARCH_BATTERY_ADC_VENDOR_CONF
#define ARCH_BATTERY_ADC_CHANNEL                ARCH_BATTERY_ADC_CHANNEL_CONF

#define ADC_REF_VOLTAGE_IN_MILLIVOLTS        1200
#define ADC_PRE_SCALING_COMPENSATION         3
#define DIODE_FWD_VOLT_DROP_MILLIVOLTS       270
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE)\
				((((ADC_VALUE) * ADC_REF_VOLTAGE_IN_MILLIVOLTS) / 1023) * ADC_PRE_SCALING_COMPENSATION)
/*---------------------------------------------------------------------------*/
static nrf_drv_adc_config_t   m_adc_config;
static nrf_drv_adc_channel_t  m_adc_channel_config;
static nrf_adc_value_t m_adc_value;
static uint16_t m_adc_result_millivolts;
static uint8_t m_adc_result_percent;
/*---------------------------------------------------------------------------*/
static __INLINE uint8_t
battery_level_in_percent(const uint16_t mvolts)
{
	uint8_t battery_level;
  if (mvolts >= 2990)  {
      battery_level = 100;
  } else if (mvolts > 2850) {
      battery_level = 100 - ((2990 - mvolts) * 32) / 100;
  } else if (mvolts > 2740) {
      battery_level = 68 - ((2850 - mvolts) * 24) / 160;
  } else if (mvolts > 2640) {
      battery_level = 18 - ((2740 - mvolts) * 12) / 300;
  } else if (mvolts > 2440) {
      battery_level = 6 - ((2640 - mvolts) * 6) / 340;
  } else {
      battery_level = 0;
  }
  return battery_level;
}
/*---------------------------------------------------------------------------*/
int
power_status(power_status_t *status)
{
	uint32_t err_code;

  // initialized pressure sensor adc configuration
  m_adc_config.interrupt_priority = ADC_CONFIG_IRQ_PRIORITY;
	m_adc_channel_config.config.config.resolution = NRF_ADC_CONFIG_RES_10BIT;
	m_adc_channel_config.config.config.input = NRF_ADC_CONFIG_SCALING_INPUT_FULL_SCALE;
	m_adc_channel_config.config.config.reference = NRF_ADC_CONFIG_REF_VBG;
	m_adc_channel_config.config.config.ain = nrf_drv_adc_gpio_to_ain(ARCH_BATTERY_ADC_PIN);
	m_adc_channel_config.p_next = NULL;

  err_code = nrf_drv_adc_init(&m_adc_config, NULL);
  if (err_code != NRF_SUCCESS) {
    PRINTF("[battery-adc] nrf_drv_saadc_channel_init error %d\n", err_code);
  }

  nrf_drv_adc_channel_enable(&m_adc_channel_config);

  //Sample SAADC on two channels
  err_code = nrf_drv_adc_sample_convert(&m_adc_channel_config, &m_adc_value);
  if (err_code != NRF_SUCCESS) {
    PRINTF("[power] nrf_drv_saadc_sample_convert error %d\n", err_code);
  }

  PRINTF("[power] adc: %d\n", m_adc_value);
  m_adc_result_millivolts = ADC_RESULT_IN_MILLI_VOLTS(m_adc_value);
  PRINTF("[power] millivolts: %d\n", m_adc_result_millivolts);

  m_adc_result_percent = battery_level_in_percent(m_adc_result_millivolts);
  PRINTF("[power] percent: %d\n", m_adc_result_percent);

  status->power_source = POWER_SOURCE_BATTERY;
  status->battery_percent = m_adc_result_percent;

  nrf_drv_adc_channel_disable(&m_adc_channel_config);

	return ENONE;
}
/*---------------------------------------------------------------------------*/
