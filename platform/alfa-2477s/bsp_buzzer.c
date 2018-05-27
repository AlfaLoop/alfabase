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
#include "bsp_buzzer.h"
#include <stdint.h>
#include <stdbool.h>
#include "frameworks/app_eventpool.h"
#include "frameworks/app_lifecycle.h"
#include "nrf_gpio.h"
#include "nrf_drv_pwm.h"
#include "nrf_delay.h"
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
int
bsp_buzzer_write(const void *buf, uint32_t len, uint32_t offset)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
static void
app_terminating(void)
{
}
/*---------------------------------------------------------------------------*/
static struct app_lifecycle_event lifecycle_event = {
	.name = "hw_bsp_buzzer",
	.terminating = app_terminating
};

#define USED_PWM(idx) (1UL << idx)
static uint8_t m_used = 0;
static nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);

static uint16_t const              m_demo1_top  = 10000;
static uint16_t const              m_demo1_step = 200;
static uint8_t                     m_demo1_phase;
static nrf_pwm_values_individual_t m_demo1_seq_values;
static nrf_pwm_sequence_t const    m_demo1_seq =
{
    .values.p_individual = &m_demo1_seq_values,
    .length              = NRF_PWM_VALUES_LENGTH(m_demo1_seq_values),
    .repeats             = 0,
    .end_delay           = 0
};
/*---------------------------------------------------------------------------*/
static void
demo1_handler(nrf_drv_pwm_evt_type_t event_type)
{
  if (event_type == NRF_DRV_PWM_EVT_FINISHED)
  {
    uint8_t channel    = m_demo1_phase >> 1;
    bool    down       = m_demo1_phase & 1;
    bool    next_phase = false;
    uint16_t * p_channels = (uint16_t *)&m_demo1_seq_values;
    uint16_t   value      = p_channels[channel];
    if (down)
    {
      value -= m_demo1_step;
      if (value == 0)
      {
        next_phase = true;
      }
    }
    else
    {
      value += m_demo1_step;
      if (value >= m_demo1_top)
      {
          next_phase = true;
      }
    }
    p_channels[channel] = value;

    if (next_phase)
    {
      if (++m_demo1_phase >= 2 * NRF_PWM_CHANNEL_COUNT)
      {
          m_demo1_phase = 0;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
demo1(void)
{
  uint32_t                   err_code;
  nrf_drv_pwm_config_t const config0 =
  {
      .output_pins =
      {
          BUZZER_PIN | NRF_DRV_PWM_PIN_INVERTED
      },
      .irq_priority = 7,
      .base_clock   = NRF_PWM_CLK_125kHz,
      .count_mode   = NRF_PWM_MODE_UP,
      .top_value    = m_demo1_top,
      .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,
      .step_mode    = NRF_PWM_STEP_AUTO
  };
  err_code = nrf_drv_pwm_init(&m_pwm0, &config0, demo1_handler);
  APP_ERROR_CHECK(err_code);
  m_used |= USED_PWM(0);

  m_demo1_seq_values.channel_0 = 0;
  m_demo1_seq_values.channel_1 = 0;
  m_demo1_seq_values.channel_2 = 0;
  m_demo1_seq_values.channel_3 = 0;
  m_demo1_phase                = 0;

  nrf_drv_pwm_simple_playback(&m_pwm0, &m_demo1_seq, 1,
                              NRF_DRV_PWM_FLAG_LOOP);
}
/*---------------------------------------------------------------------------*/
static void
demo2(void)
{
  uint32_t                   err_code;
    /*
     * This demo plays back two concatenated sequences:
     * - Sequence 0: Light intensity is increased in 25 steps during one second.
     * - Sequence 1: LED blinks twice (100 ms off, 100 ms on), then stays off
     *   for 200 ms.
     * The same output is generated on all 4 channels (LED 1 - LED 4).
     * The playback is repeated in a loop.
     */

    // [local constant parameters]
    enum
    {
        TOP        = 10000,
        STEP_COUNT = 25
    };

    nrf_drv_pwm_config_t const config0 =
    {
        .output_pins =
        {
            BUZZER_PIN | NRF_DRV_PWM_PIN_INVERTED, // channel 0
        },
        .irq_priority = 7,
        .base_clock   = NRF_PWM_CLK_500kHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = TOP,
        .load_mode    = NRF_PWM_LOAD_COMMON,
        .step_mode    = NRF_PWM_STEP_AUTO
    };
    err_code = nrf_drv_pwm_init(&m_pwm0, &config0, NULL);
    APP_ERROR_CHECK(err_code);
    m_used |= USED_PWM(0);

    // This array cannot be allocated on stack (hence "static") and it must
    // be in RAM.
    static nrf_pwm_values_common_t seq0_values[STEP_COUNT];
    nrf_pwm_sequence_t const       seq0 =
    {
        .values.p_common = seq0_values,
        .length          = NRF_PWM_VALUES_LENGTH(seq0_values),
        .repeats         = 1,
        .end_delay       = 0
    };
    uint16_t value = 0;
    uint16_t step  = TOP / STEP_COUNT;
    uint8_t  i;

    for (i = 0; i < STEP_COUNT; ++i)
    {
        value         += step;
        seq0_values[i] = value;
    }

    // This array cannot be allocated on stack (hence "static") and it must
    // be in RAM (hence no "const", though its content is not changed).
    static nrf_pwm_values_common_t  /*const*/ seq1_values[] =
    {
        0,
        0x8000,
        0,
        0x8000,
        0,
        0
    };
    nrf_pwm_sequence_t const seq1 =
    {
        .values.p_common = seq1_values,
        .length          = NRF_PWM_VALUES_LENGTH(seq1_values),
        .repeats         = 4,
        .end_delay       = 0
    };

    nrf_drv_pwm_complex_playback(&m_pwm0, &seq0, &seq1, 1,
                                 NRF_DRV_PWM_FLAG_LOOP);
}
/*---------------------------------------------------------------------------*/
static void
demo3(void)
{
    /*
     * This demo uses only one channel, which is reflected on LED 1.
     * The LED blinks three times (200 ms on, 200 ms off), then it stays off
     * for one second.
     * This scheme is performed three times before the peripheral is stopped.
     */

    uint32_t                   err_code;
    nrf_drv_pwm_config_t const config0 =
    {
        .output_pins =
        {
            BUZZER_PIN | NRF_DRV_PWM_PIN_INVERTED
        },
        .irq_priority = 7,
        .base_clock   = NRF_PWM_CLK_125kHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = 25000,
        .load_mode    = NRF_PWM_LOAD_COMMON,
        .step_mode    = NRF_PWM_STEP_AUTO
    };
    err_code = nrf_drv_pwm_init(&m_pwm0, &config0, NULL);
    APP_ERROR_CHECK(err_code);
    m_used |= USED_PWM(0);

    // This array cannot be allocated on stack (hence "static") and it must
    // be in RAM (hence no "const", though its content is not changed).
    static uint16_t  /*const*/ seq_values[] =
    {
        0x8000,
        0,
        0x8000,
        0,
        0x8000,
        0
    };
    nrf_pwm_sequence_t const seq =
    {
        .values.p_common = seq_values,
        .length          = NRF_PWM_VALUES_LENGTH(seq_values),
        .repeats         = 0,
        .end_delay       = 4
    };

    nrf_drv_pwm_simple_playback(&m_pwm0, &seq, 3, NRF_DRV_PWM_FLAG_STOP);
}
/*---------------------------------------------------------------------------*/
int
bsp_buzzer_init(void)
{
  uint32_t                   err_code;

	app_lifecycle_register(&lifecycle_event);

  nrf_gpio_cfg_output(BUZZER_PIN);
  nrf_gpio_pin_clear(BUZZER_PIN);

  // demo1();
  // demo2();
  // demo3();

  // for (int i = 0; i < 3000000; i++) {
  //   nrf_gpio_pin_toggle(BUZZER_PIN);
  //   nrf_delay_us(450);
  //   // nrf_delay_ms(10);
  // }
  //
  // nrf_gpio_pin_clear(BUZZER_PIN);
  //
  // nrf_delay_ms(1000);
  //
  // for (int i = 0; i < 300; i++) {
  //   nrf_gpio_pin_toggle(BUZZER_PIN);
  //   nrf_delay_us(270);
  //   // nrf_delay_ms(10);
  // }
  // while (app_pwm_channel_duty_set(&PWM1, 0, 50) == NRF_ERROR_BUSY);
  // APP_PWM_DEFAULT_CONFIG_1CH(2000L,pin_number)
}
/*---------------------------------------------------------------------------*/
