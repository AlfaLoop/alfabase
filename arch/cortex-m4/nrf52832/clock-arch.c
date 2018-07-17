/*
 * Copyright (c) 2009, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
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
#include <stdint.h>
#include <stdbool.h>
#include "nrf_delay.h"

/* Scheduler includes. */
#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"
#endif

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

#define APP_TIMER_TICKS(MS) (uint32_t)ROUNDED_DIV((MS)*configTICK_RATE_HZ,1000)

/* Check if RTC FreeRTOS version is used */
#if configTICK_SOURCE != FREERTOS_USE_RTC
#error app_timer in FreeRTOS variant have to be used with RTC tick source configuration. Default configuration have to be used in other case.
#endif

/* Check if freeRTOS timers are activated */
#if configUSE_TIMERS == 0
    #error app_timer for freeRTOS requires configUSE_TIMERS option to be activated.
#endif

/**
 * @brief Waiting time for the timer queue
 *
 * Number of system ticks to wait for the timer queue to put the message.
 * It is strongly recommended to set this to the value bigger than 1.
 * In other case if timer message queue is full - any operation on timer may fail.
 * @note
 * Timer functions called from interrupt context would never wait.
 */
#define APP_TIMER_WAIT_FOR_QUEUE 2

extern TaskHandle_t g_contiki_thread;
static TimerHandle_t m_contiki_clock_timer;

/* FreeRTOS Timer is used to implement Contiki's clock */


/** \brief Function to return the number of ticks of the clock
 *  \return The current os clock ticks
 */
static clock_time_t
os_tick_time(void)
{
	return xTaskGetTickCount();
}

/**
 * @brief Internal callback function for the system timer
 *
 * Internal function that is called from the system timer.
 * It gets our parameter from timer data and sends it to user function.
 * @param[in] xTimer Timer handler
 */
static void
clock_timer_callback(TimerHandle_t timer_handle)
{
  if (timer_handle == m_contiki_clock_timer) {
    etimer_request_poll();
    xTaskNotifyGive( g_contiki_thread );
  }
}

// specific the Contiki clock initialization with FreeRTOS timer.
void
clock_init(void)
{
	m_contiki_clock_timer = xTimerCreate(" ", 400, pdFALSE, NULL, clock_timer_callback);
	if (m_contiki_clock_timer == NULL){
		PRINTF("[clock arch] clock init error\n");
	}
}

// Get the current clock time.
CCIF clock_time_t
clock_time(void)
{
	return os_tick_time();
}

// Get the current value of the platform seconds.
CCIF unsigned long
clock_seconds(void)
{
	return os_tick_time() / CLOCK_SECOND;
}
/**
 * \brief Set the value of the platform seconds.
 * \param sec   The value to set for seconds.
 *
 */
void
clock_set_seconds(unsigned long sec)
{

}

/**
 * \brief Wait for a given number of ticks.
 * \param t   How many ticks.
 *
 */
void
clock_wait(clock_time_t t)
{
  if(t) {
    vTaskDelay(t);
  }
}

/**
 * \brief Delay a given number of microseconds.
 * 		  This implementation uses NOP statements of CPU to while
 * 		  alway the required amount of time. So, it is not power
 * 		  efficient and accurate.
 * \param dt How many milliseconds to delay.
 *
 * \note Interrupts could increase the delay by a variable amount.
 */
void
clock_delay(unsigned int dt)
{
  nrf_delay_ms(dt);
}

/**
 * \brief Delay a given number of microseconds.
 *		  This implementation uses NOP statements of CPU to while
 * 		  alway the required amount of time. So, it is not power
 * 		  efficient and accurate.
 * \param dt   How many microseconds to delay.
 *
 * \note Interrupts could increase the delay by a variable amount.
 */
void
clock_delay_usec(uint16_t dt)
{
  nrf_delay_us(dt);
}


uint32_t
clock_time_to_tick(clock_time_t time)
{
  return APP_TIMER_TICKS(time);
}

/** \brief Function initializes code to call etimer poll based on expiration time received
 *			The counter compare interrupt is initialized so that the interrupt occurs when
 *			the expiration occurs and etimer poll is called from the ISR.
 * \param expiration_time The value of current_clock at which etimer expiration occurs
 * \warning Since the RTC is a 24 bit counter, the expiration time must be less that 2^24
 * 			even though the current_clock is 32 bit variable.
 */
void
clock_update_expiration_time(clock_time_t expiration_time)
{
  // Get the Interrupt Program Status Register (IPSR) status.
  // If the current statu is not in Thread mode (0)
  uint32_t curr_tick = os_tick_time();
  uint32_t timeout_offset = expiration_time - curr_tick;
  uint32_t timer_period;

  timer_period = timeout_offset;
  if (timer_period == 0) {
    return;
  }
  if (__get_IPSR() != 0) {
    PRINTF("[clock arch] __get_IPSR \n");
    BaseType_t yieldReq = pdFALSE;
    if (xTimerChangePeriodFromISR(m_contiki_clock_timer, timer_period,
      &yieldReq) != pdPASS)
    {
        return;
    }
    if ( xTimerStartFromISR(m_contiki_clock_timer, &yieldReq) != pdPASS )
    {
        return;
    }

    portYIELD_FROM_ISR(yieldReq);
  } else {
      if (xTimerChangePeriod(m_contiki_clock_timer, timer_period, APP_TIMER_WAIT_FOR_QUEUE)
        != pdPASS)
      {
          return;
      }
      if (xTimerStart(m_contiki_clock_timer, APP_TIMER_WAIT_FOR_QUEUE) != pdPASS) {
          return;
      }
  }
}
