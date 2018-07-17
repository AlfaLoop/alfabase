/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 */

#include "ble_radio_notification.h"
#include <stdlib.h>

/* Scheduler includes. */
#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#endif


static bool                                 m_radio_active = false;  /**< Current radio state. */
static ble_radio_notification_evt_handler_t m_evt_handler  = NULL;   /**< Application event handler for handling Radio Notification events. */
extern TaskHandle_t g_contiki_thread;

void SWI1_IRQHandler(void)
{
	BaseType_t yield_req = pdFALSE;

	m_radio_active = !m_radio_active;
	if (m_evt_handler != NULL)
	{
			m_evt_handler(m_radio_active);
	}

	vTaskNotifyGiveFromISR(g_contiki_thread, &yield_req);

	/* Switch the task if required. */
	portYIELD_FROM_ISR(yield_req);
}

uint32_t ble_radio_notification_init(uint32_t                             irq_priority,
                                     uint8_t    distance,
                                     ble_radio_notification_evt_handler_t evt_handler)
{
    uint32_t err_code;

    m_evt_handler = evt_handler;

    // Initialize Radio Notification software interrupt
    err_code = sd_nvic_ClearPendingIRQ(SWI1_IRQn);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = sd_nvic_SetPriority(SWI1_IRQn, irq_priority);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = sd_nvic_EnableIRQ(SWI1_IRQn);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Configure the event
    return sd_radio_notification_cfg_set(NRF_RADIO_NOTIFICATION_TYPE_INT_ON_BOTH, distance);
}
