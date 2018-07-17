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
#include "battery-adc.h"
#include <stdint.h>
#include <stdbool.h>
#include "nrf_drv_adc.h"
#include "frameworks/app_eventpool.h"
#include "frameworks/app_lifecycle.h"
#include "nrf_gpio.h"
#include "sys/clock.h"
#include "sys/ctimer.h"
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

static uint16_t m_cfg_sample_rate = 5000;
static struct ctimer m_battery_update_timer;
static nrf_drv_adc_config_t   m_adc_config;
static nrf_drv_adc_channel_t  m_adc_channel_config;
static nrf_adc_value_t m_adc_value;
static uint16_t m_adc_result_millivolts;
static uint8_t m_adc_result_percent;
static bool m_battery_monitor_active;
/*---------------------------------------------------------------------------*/
static __INLINE uint8_t
battery_level_in_percent(const uint16_t mvolts)
{
  uint8_t battery_level;
  if (mvolts >= 3000)  {
      battery_level = 100;
  } else if (mvolts > 2900) {
      battery_level = 100 - ((3000 - mvolts) * 58) / 100;
  }else if (mvolts > 2740) {
      battery_level = 42 - ((2900 - mvolts) * 24) / 160;
  } else if (mvolts > 2440) {
      battery_level = 18 - ((2740 - mvolts) * 12) / 300;
  }else if (mvolts > 2100) {
      battery_level = 6 - ((2440 - mvolts) * 6) / 340;
  } else {
      battery_level = 0;
  }
  return battery_level;
}
/*---------------------------------------------------------------------------*/
static void
uninit_battery_sensor_input(void)
{
	nrf_drv_adc_channel_disable(&m_adc_channel_config);
}
/*---------------------------------------------------------------------------*/
static void
init_battery_sensor_input(void)
{
	m_adc_config.interrupt_priority = ADC_CONFIG_IRQ_PRIORITY;
	m_adc_channel_config.config.config.resolution = NRF_ADC_CONFIG_RES_10BIT;
	m_adc_channel_config.config.config.input = NRF_ADC_CONFIG_SCALING_INPUT_FULL_SCALE;
	m_adc_channel_config.config.config.reference = NRF_ADC_CONFIG_REF_VBG;
	m_adc_channel_config.config.config.ain = nrf_drv_adc_gpio_to_ain(ARCH_BATTERY_ADC_PIN);
	m_adc_channel_config.p_next = NULL;
}
/*---------------------------------------------------------------------------*/
static void
battery_level_update(void)
{
	uint32_t err_code;
	if (!m_battery_monitor_active)
		return;

	//Sample SAADC on two channels
	err_code = nrf_drv_adc_sample_convert(&m_adc_channel_config, &m_adc_value);
	if (err_code != NRF_SUCCESS) {
		PRINTF("[battery-adc] nrf_drv_saadc_sample_convert error %d\n", err_code);
	}

  PRINTF("[battery-adc] adc: %d\n", m_adc_value);
	m_adc_result_millivolts = ADC_RESULT_IN_MILLI_VOLTS(m_adc_value);
	PRINTF("[battery-adc] millivolts: %d\n", m_adc_result_millivolts);

	m_adc_result_percent = battery_level_in_percent(m_adc_result_millivolts);          //Transform the millivolts value into battery level percent.
  PRINTF("[battery-adc] percent: %d\n", m_adc_result_percent);

	SensorEvent event;
	memset(&event, 0, sizeof(SensorEvent));
	event.type = TYPE_BATTERY;
	event.data_len = 1;
	event.data[0] = (uint8_t)(m_adc_result_percent);
	event.timestamp = clock_time();

	AppIrqEvent appIrqEvent;
	appIrqEvent.event_type = APP_SENSOR_EVENT;
	memcpy(&appIrqEvent.params.sensorEvent, &event, sizeof(SensorEvent));
	appIrqEvent.callback = sensor_api_irq;
	// push the timer callback event to user interrupt routin task
	xQueueSend( xAppIrqQueueHandle,  &appIrqEvent, ( TickType_t ) 0 );
  ctimer_set(&m_battery_update_timer, m_cfg_sample_rate, battery_level_update, (void *)NULL);
}
/*---------------------------------------------------------------------------*/
static void
adc_event_handler(nrf_drv_adc_evt_t const * p_event)
{

}
/*---------------------------------------------------------------------------*/
static void
parse_rate(SensorRate rate)
{
	switch (rate) {
		case RATE_SLOW:
		m_cfg_sample_rate = 30000;
		break;
		case RATE_NORMAL:
		m_cfg_sample_rate = 10000;
		break;
		case RATE_FAST:
		m_cfg_sample_rate = 5000;
		break;
		case RATE_FASTED:
		m_cfg_sample_rate = 2000;
		break;
		default:
		m_cfg_sample_rate = 5000;
	}
}
/*---------------------------------------------------------------------------*/
bool
battery_ossensor_is_supported(void)
{
  return true;
}
/*---------------------------------------------------------------------------*/
int
battery_ossensor_open(SensorParams *params)
{
	uint32_t err_code;
	if (m_battery_monitor_active) {
		return EINVALSTATE;
	}
	if (params == NULL) {
		return ENULLP;
	}
	parse_rate(params->rate);

	err_code = nrf_drv_adc_init(&m_adc_config, NULL);
	if (err_code != NRF_SUCCESS) {
		PRINTF("[battery-adc] nrf_drv_saadc_channel_init error %d\n", err_code);
	}

	nrf_drv_adc_channel_enable(&m_adc_channel_config);

	ctimer_set(&m_battery_update_timer, m_cfg_sample_rate, battery_level_update, (void *)NULL);
	m_battery_monitor_active = true;

  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
battery_ossensor_close(void)
{
	if (!m_battery_monitor_active)
		return EINVALSTATE;

	ctimer_stop(&m_battery_update_timer);
	m_battery_monitor_active = false;
	uninit_battery_sensor_input();
  return ENONE;
}
/*---------------------------------------------------------------------------*/
char*
battery_ossensor_get_vendor(void)
{
  const char *vendor = ARCH_BATTERY_ADC_VENDOR;
  return &vendor;
}
/*---------------------------------------------------------------------------*/
static void
app_terminating(void)
{
	if (m_battery_monitor_active) {
		ctimer_stop(&m_battery_update_timer);
		m_battery_monitor_active = false;
		uninit_battery_sensor_input();
	}
}
/*---------------------------------------------------------------------------*/
static struct app_lifecycle_event lifecycle_event = {
	.name = "battery-adc",
	.terminating = app_terminating
};
/*---------------------------------------------------------------------------*/
int
sensor_battery_arch_init(void)
{
	uint32_t err_code;
	m_battery_monitor_active = false;

	app_lifecycle_register(&lifecycle_event);

	// initialized pressure sensor adc configuration
	init_battery_sensor_input();
}
/*---------------------------------------------------------------------------*/
