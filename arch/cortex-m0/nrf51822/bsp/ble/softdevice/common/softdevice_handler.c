/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#include "sdk_config.h"
#include "softdevice_handler.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "app_error.h"
#include "nrf_assert.h"
#include "nrf_nvic.h"
#include "nrf.h"
#include "sdk_common.h"
#include "libs/util/byteorder.h"
#if CLOCK_ENABLED
#include "nrf_drv_clock.h"
#endif

/* Scheduler includes. */
#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#endif


#define NRF_LOG_MODULE_NAME "SDH"
#include "nrf_log.h"
#if defined(ANT_STACK_SUPPORT_REQD) && defined(BLE_STACK_SUPPORT_REQD)
    #include "ant_interface.h"
#elif defined(ANT_STACK_SUPPORT_REQD)
    #include "ant_interface.h"
#elif defined(BLE_STACK_SUPPORT_REQD)
    #include "ble.h"
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
#define RAM_START_ADDRESS         0x20000000
#define SOFTDEVICE_EVT_IRQ        SD_EVT_IRQn       /**< SoftDevice Event IRQ number. Used for both protocol events and SoC events. */
#define SOFTDEVICE_EVT_IRQHandler SD_EVT_IRQHandler
#define RAM_TOTAL_SIZE            ((NRF_FICR->INFO.RAM) * 1024)
#define RAM_END_ADDRESS           (RAM_START_ADDRESS + RAM_TOTAL_SIZE)


#define SOFTDEVICE_VS_UUID_COUNT       0
#define SOFTDEVICE_GATTS_ATTR_TAB_SIZE BLE_GATTS_ATTR_TAB_SIZE_DEFAULT
#define SOFTDEVICE_GATTS_SRV_CHANGED   0
#define SOFTDEVICE_PERIPH_CONN_COUNT   1
#define SOFTDEVICE_CENTRAL_CONN_COUNT  4
#define SOFTDEVICE_CENTRAL_SEC_COUNT   1

// extern SemaphoreHandle_t g_contiki_task_semephore;
extern TaskHandle_t g_contiki_thread;

static softdevice_evt_schedule_func_t m_evt_schedule_func;              /**< Pointer to function for propagating SoftDevice events to the scheduler. */

static volatile bool                  m_softdevice_enabled = false;     /**< Variable to indicate whether the SoftDevice is enabled. */
static volatile bool                  m_suspended;                      /**< Current state of the event handler. */
#ifdef BLE_STACK_SUPPORT_REQD
// The following three definitions is needed only if BLE events are needed to be pulled from the stack.
static uint8_t                      * mp_ble_evt_buffer;                /**< Buffer for receiving BLE events from the SoftDevice. */
static uint16_t                       m_ble_evt_buffer_size;            /**< Size of BLE event buffer. */
static ble_evt_handler_t              m_ble_evt_handler;                /**< Application event handler for handling BLE events. */
#endif

#ifdef ANT_STACK_SUPPORT_REQD
// The following two definitions are needed only if ANT events are needed to be pulled from the stack.
static ant_evt_t                      m_ant_evt_buffer;                 /**< Buffer for receiving ANT events from the SoftDevice. */
static ant_evt_handler_t              m_ant_evt_handler;                /**< Application event handler for handling ANT events.  */
#endif

static sys_evt_handler_t              m_sys_evt_handler;                /**< Application event handler for handling System (SOC) events.  */

#if (CLOCK_ENABLED && defined(SOFTDEVICE_PRESENT))
/**
 * @brief Check the selected ISR state.
 *
 * Implementation of a function that checks the current IRQ state.
 *
 * @param[in] IRQn External interrupt number. Value cannot be negative.
 *
 * @retval true  Selected IRQ is enabled.
 * @retval false Selected IRQ is disabled.
 */
static inline bool isr_enable_check(IRQn_Type IRQn)
{
    return 0 != (NVIC->ISER[(((uint32_t)(int32_t)IRQn) >> 5UL)] & (uint32_t)(1UL << (((uint32_t)(int32_t)IRQn) & 0x1FUL)));
}
#endif

/**@brief       Callback function for asserts in the SoftDevice.
 *
 * @details     A pointer to this function will be passed to the SoftDevice. This function will be
 *              called by the SoftDevice if certain unrecoverable errors occur within the
 *              application or SoftDevice.
 *
 *              See @ref nrf_fault_handler_t for more details.
 *
 * @param[in] id    Fault identifier. See @ref NRF_FAULT_IDS.
 * @param[in] pc    The program counter of the instruction that triggered the fault.
 * @param[in] info  Optional additional information regarding the fault. Refer to each fault
 *                  identifier for details.
 */
void softdevice_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
  PRINTF("[softdevice] fault id %d pc %d info %d\n", id, pc, info);
  app_error_fault_handler(id, pc, info);
}

void intern_softdevice_events_execute(void)
{
    if (!m_softdevice_enabled){
        // SoftDevice not enabled. This can be possible if the SoftDevice was enabled by the
        // application without using this module's API (i.e softdevice_handler_init)
        return;
    }
#if CLOCK_ENABLED
    bool no_more_soc_evts = false;
#else
    bool no_more_soc_evts = (m_sys_evt_handler == NULL);
#endif
#ifdef BLE_STACK_SUPPORT_REQD
    bool no_more_ble_evts = (m_ble_evt_handler == NULL);
#endif
#ifdef ANT_STACK_SUPPORT_REQD
    bool no_more_ant_evts = (m_ant_evt_handler == NULL);
#endif

    for (;;)
    {
        uint32_t err_code;

        if (!no_more_soc_evts)
        {
            if (m_suspended)
            {
                // Cancel pulling next event if event handler was suspended by user.
                return;
            }

            uint32_t evt_id;

            // Pull event from SOC.
            err_code = sd_evt_get(&evt_id);

            if (err_code == NRF_ERROR_NOT_FOUND)
            {
                no_more_soc_evts = true;
            }
            else if (err_code != NRF_SUCCESS)
            {
                APP_ERROR_HANDLER(err_code);
            }
            else
            {
                // Call application's SOC event handler.
#if (CLOCK_ENABLED && defined(SOFTDEVICE_PRESENT))
                nrf_drv_clock_on_soc_event(evt_id);
                if (m_sys_evt_handler)
                {
                    m_sys_evt_handler(evt_id);
                }
#else
                m_sys_evt_handler(evt_id);
#endif
            }
        }

#ifdef BLE_STACK_SUPPORT_REQD
        // Fetch BLE Events.
        if (!no_more_ble_evts)
        {
            if (m_suspended)
            {
                // Cancel pulling next event if event handler was suspended by user.
                return;
            }

            // Pull event from stack
            uint16_t evt_len = m_ble_evt_buffer_size;

            err_code = sd_ble_evt_get(mp_ble_evt_buffer, &evt_len);
            if (err_code == NRF_ERROR_NOT_FOUND)
            {
                no_more_ble_evts = true;
            }
            else if (err_code != NRF_SUCCESS)
            {
                APP_ERROR_HANDLER(err_code);
            }
            else
            {
                // Call application's BLE stack event handler.
                m_ble_evt_handler((ble_evt_t *)mp_ble_evt_buffer);
            }
        }
#endif

#ifdef ANT_STACK_SUPPORT_REQD
        // Fetch ANT Events.
        if (!no_more_ant_evts)
        {
            if (m_suspended)
            {
                // Cancel pulling next event if event handler was suspended by user.
                return;
            }

            // Pull event from stack
            err_code = sd_ant_event_get(&m_ant_evt_buffer.channel,
                                        &m_ant_evt_buffer.event,
                                        m_ant_evt_buffer.msg.evt_buffer);
            if (err_code == NRF_ERROR_NOT_FOUND)
            {
                no_more_ant_evts = true;
            }
            else if (err_code != NRF_SUCCESS)
            {
                APP_ERROR_HANDLER(err_code);
            }
            else
            {
                // Call application's ANT stack event handler.
                m_ant_evt_handler(&m_ant_evt_buffer);
            }
        }
#endif

        if (no_more_soc_evts)
        {
            // There are no remaining System (SOC) events to be fetched from the SoftDevice.
#if defined(ANT_STACK_SUPPORT_REQD) && defined(BLE_STACK_SUPPORT_REQD)
            // Check if there are any remaining BLE and ANT events.
            if (no_more_ble_evts && no_more_ant_evts)
            {
                break;
            }
#elif defined(BLE_STACK_SUPPORT_REQD)
            // Check if there are any remaining BLE events.
            if (no_more_ble_evts)
            {
                break;
            }
#elif defined(ANT_STACK_SUPPORT_REQD)
            // Check if there are any remaining ANT events.
            if (no_more_ant_evts)
            {
                break;
            }
#else
            // No need to check for BLE or ANT events since there is no support for BLE and ANT
            // required.
            break;
#endif
        }
    }
}

bool softdevice_handler_is_enabled(void)
{
    return m_softdevice_enabled;
}

uint32_t softdevice_handler_init(nrf_clock_lf_cfg_t *           p_clock_lf_cfg,
                                 void *                         p_ble_evt_buffer,
                                 uint16_t                       ble_evt_buffer_size,
                                 softdevice_evt_schedule_func_t evt_schedule_func)
{
    uint32_t err_code;

    // Save configuration.
#if defined (BLE_STACK_SUPPORT_REQD)
    // Check that buffer is not NULL.
    if (p_ble_evt_buffer == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    // Check that buffer is correctly aligned.
    if (!is_word_aligned(p_ble_evt_buffer))
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    mp_ble_evt_buffer     = (uint8_t *)p_ble_evt_buffer;
    m_ble_evt_buffer_size = ble_evt_buffer_size;
#else
    // The variables p_ble_evt_buffer and ble_evt_buffer_size is not needed if BLE Stack support
    // is not required.
    UNUSED_PARAMETER(p_ble_evt_buffer);
    UNUSED_PARAMETER(ble_evt_buffer_size);
#endif

    m_evt_schedule_func = evt_schedule_func;

    // Initialize SoftDevice.
#if (CLOCK_ENABLED && defined(SOFTDEVICE_PRESENT))
    bool power_clock_isr_enabled = isr_enable_check(POWER_CLOCK_IRQn);
    if (power_clock_isr_enabled)
    {
        NVIC_DisableIRQ(POWER_CLOCK_IRQn);
    }
#endif
#if defined(S212) || defined(S332)
    err_code = sd_softdevice_enable(p_clock_lf_cfg, softdevice_fault_handler, ANT_LICENSE_KEY);
#else
    err_code = sd_softdevice_enable(p_clock_lf_cfg, softdevice_fault_handler);
#endif

    if (err_code != NRF_SUCCESS)
    {
#if (CLOCK_ENABLED && defined(SOFTDEVICE_PRESENT))
        if (power_clock_isr_enabled)
        {
            NVIC_EnableIRQ(POWER_CLOCK_IRQn);
        }
#endif
        return err_code;
    }

    m_softdevice_enabled = true;
#if (CLOCK_ENABLED && defined(SOFTDEVICE_PRESENT))
    nrf_drv_clock_on_sd_enable();
#endif

    // Enable BLE event interrupt (interrupt priority has already been set by the stack).
#ifdef SOFTDEVICE_PRESENT
    return sd_nvic_EnableIRQ((IRQn_Type)SOFTDEVICE_EVT_IRQ);
#else
    //In case of Serialization NVIC must be accessed directly.
    NVIC_EnableIRQ(SOFTDEVICE_EVT_IRQ);
    return NRF_SUCCESS;
#endif
}


uint32_t softdevice_handler_sd_disable(void)
{
    uint32_t err_code = sd_softdevice_disable();
#if (CLOCK_ENABLED && defined(SOFTDEVICE_PRESENT))
    if (err_code == NRF_SUCCESS)
    {
        m_softdevice_enabled = false;
        nrf_drv_clock_on_sd_disable();
    }
#else
    m_softdevice_enabled = !(err_code == NRF_SUCCESS);
#endif
    return err_code;
}

#ifdef BLE_STACK_SUPPORT_REQD
uint32_t softdevice_ble_evt_handler_set(ble_evt_handler_t ble_evt_handler)
{
    VERIFY_PARAM_NOT_NULL(ble_evt_handler);

    m_ble_evt_handler = ble_evt_handler;

    return NRF_SUCCESS;
}
#endif


#ifdef ANT_STACK_SUPPORT_REQD
uint32_t softdevice_ant_evt_handler_set(ant_evt_handler_t ant_evt_handler)
{
    VERIFY_PARAM_NOT_NULL(ant_evt_handler);

    m_ant_evt_handler = ant_evt_handler;

    return NRF_SUCCESS;
}
#endif


uint32_t softdevice_sys_evt_handler_set(sys_evt_handler_t sys_evt_handler)
{
    VERIFY_PARAM_NOT_NULL(sys_evt_handler);

    m_sys_evt_handler = sys_evt_handler;

    return NRF_SUCCESS;
}


/**@brief   Function for handling the Application's BLE Stack events interrupt.
 *
 * @details This function is called whenever an event is ready to be pulled.
 */
void SOFTDEVICE_EVT_IRQHandler(void)
{
  //BaseType_t yield_req = pdFALSE;
  // The returned value may be safely ignored, if error is returned it only means that
  // the semaphore is already given (raised).
  /*
  UNUSED_VARIABLE(xSemaphoreGiveFromISR(g_contiki_task_semephore, &yield_req));
  portYIELD_FROM_ISR(yield_req);
  intern_softdevice_events_execute();
  */
  BaseType_t yield_req = pdFALSE;

  intern_softdevice_events_execute();

  vTaskNotifyGiveFromISR(g_contiki_thread, &yield_req);

  /* Switch the task if required. */
  portYIELD_FROM_ISR(yield_req);
}

void softdevice_handler_suspend()
{
#ifdef SOFTDEVICE_PRESENT
    ret_code_t err_code = sd_nvic_DisableIRQ((IRQn_Type)SOFTDEVICE_EVT_IRQ);
    APP_ERROR_CHECK(err_code);
#else
    NVIC_DisableIRQ(SOFTDEVICE_EVT_IRQ);
#endif
    m_suspended = true;
    return;
}

void softdevice_handler_resume()
{
    if (!m_suspended) return;
    m_suspended = false;

#ifdef SOFTDEVICE_PRESENT
    ret_code_t err_code;

    // Force calling ISR again to make sure that events not pulled previously
    // has been processed.
    err_code = sd_nvic_SetPendingIRQ((IRQn_Type)SOFTDEVICE_EVT_IRQ);
    APP_ERROR_CHECK(err_code);
    err_code = sd_nvic_EnableIRQ((IRQn_Type)SOFTDEVICE_EVT_IRQ);
    APP_ERROR_CHECK(err_code);
#else
    NVIC_SetPendingIRQ((IRQn_Type)SOFTDEVICE_EVT_IRQ);
    NVIC_EnableIRQ(SOFTDEVICE_EVT_IRQ);
#endif

    return;
}

bool softdevice_handler_is_suspended()
{
    return m_suspended;
}

#if defined(BLE_STACK_SUPPORT_REQD)
uint32_t softdevice_enable_get_default_config(uint8_t central_links_count,
                                              uint8_t periph_links_count,
                                              ble_enable_params_t * p_ble_enable_params)
{
    memset(p_ble_enable_params, 0, sizeof(ble_enable_params_t));
    p_ble_enable_params->common_enable_params.vs_uuid_count   = 1;
    p_ble_enable_params->gatts_enable_params.attr_tab_size    = SOFTDEVICE_GATTS_ATTR_TAB_SIZE;
    p_ble_enable_params->gatts_enable_params.service_changed  = SOFTDEVICE_GATTS_SRV_CHANGED;
    p_ble_enable_params->gap_enable_params.periph_conn_count  = periph_links_count;
    p_ble_enable_params->gap_enable_params.central_conn_count = central_links_count;
    if (p_ble_enable_params->gap_enable_params.central_conn_count != 0)
    {
        p_ble_enable_params->gap_enable_params.central_sec_count  = SOFTDEVICE_CENTRAL_SEC_COUNT;
    }

    return NRF_SUCCESS;
}


static inline uint32_t ram_total_size_get(void)
{
#ifdef NRF51
    uint32_t size_ram_blocks = (uint32_t)NRF_FICR->SIZERAMBLOCKS;
    uint32_t total_ram_size = size_ram_blocks;
    total_ram_size = total_ram_size * (NRF_FICR->NUMRAMBLOCK);
    return total_ram_size;
#elif defined (NRF52)
    return RAM_TOTAL_SIZE;
#endif /* NRF51 */
}

/*lint --e{528} -save suppress 528: symbol not referenced */
/**@brief   Function for finding the end address of the RAM.
 *
 * @retval  ram_end_address  Address of the end of the RAM.
 */
static inline uint32_t ram_end_address_get(void)
{
    uint32_t ram_end_address = (uint32_t)RAM_START_ADDRESS;
    ram_end_address+= ram_total_size_get();
    return ram_end_address;
}
/*lint -restore*/

/*lint --e{10} --e{19} --e{27} --e{40} --e{529} -save suppress Error 27: Illegal character */
uint32_t sd_check_ram_start(uint32_t sd_req_ram_start)
{
#if (defined(S130) || defined(S132) || defined(S332))
#if defined ( __CC_ARM )
    extern uint32_t Image$$RW_IRAM1$$Base;
    const volatile uint32_t ram_start = (uint32_t) &Image$$RW_IRAM1$$Base;
#elif defined ( __ICCARM__ )
    extern uint32_t __ICFEDIT_region_RAM_start__;
    volatile uint32_t ram_start = (uint32_t) &__ICFEDIT_region_RAM_start__;
#elif defined   ( __GNUC__ )
    extern uint32_t __data_start__;
    volatile uint32_t ram_start = (uint32_t) &__data_start__;
#endif//__CC_ARM
    if (ram_start != sd_req_ram_start)
    {
        NRF_LOG_WARNING("RAM START ADDR 0x%x should be adjusted to 0x%x\r\n",
                  ram_start,
                  sd_req_ram_start);
        NRF_LOG_WARNING("RAM SIZE should be adjusted to 0x%x \r\n",
                ram_end_address_get() - sd_req_ram_start);
        return NRF_SUCCESS;
    }
#endif//defined(S130) || defined(S132) || defined(S332)
    return NRF_SUCCESS;
}

uint32_t softdevice_enable(ble_enable_params_t * p_ble_enable_params)
{
#if (defined(S130) || defined(S132) || defined(S332))
    uint32_t err_code;
    uint32_t app_ram_base;

#if defined ( __CC_ARM )
    extern uint32_t Image$$RW_IRAM1$$Base;
    const volatile uint32_t ram_start = (uint32_t) &Image$$RW_IRAM1$$Base;
#elif defined ( __ICCARM__ )
    extern uint32_t __ICFEDIT_region_RAM_start__;
    volatile uint32_t ram_start = (uint32_t) &__ICFEDIT_region_RAM_start__;
#elif defined   ( __GNUC__ )
    extern uint32_t __data_start__;
    volatile uint32_t ram_start = (uint32_t) &__data_start__;
#endif

    app_ram_base = ram_start;
    NRF_LOG_INFO("sd_ble_enable: RAM START at 0x%x\r\n",
                    app_ram_base);
    err_code = sd_ble_enable(p_ble_enable_params, &app_ram_base);

    if (app_ram_base != ram_start)
    {
        NRF_LOG_WARNING("sd_ble_enable: app_ram_base should be adjusted to 0x%x\r\n",
                app_ram_base);
        NRF_LOG_WARNING("ram size should be adjusted to 0x%x \r\n",
                ram_end_address_get() - app_ram_base);
    }
    else if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("sd_ble_enable: error 0x%x\r\n", err_code);
    }
    return err_code;
#else
    return NRF_SUCCESS;
#endif   //defined(S130) || defined(S132) || defined(S332)

}
/*lint -restore*/

#endif //BLE_STACK_SUPPORT_REQD
