  /* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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
#include "nrf_nvic.h"
#include "nrf_error.h"
/**@brief Variable keeping the state for the SoftDevice NVIC module. This must be declared in an
 * application source file. */
nrf_nvic_state_t nrf_nvic_state;


/**@} */

/**@addtogroup NRF_NVIC_INTERNAL_FUNCTIONS SoftDevice NVIC internal functions
 * @{ */

/**@brief Disables IRQ interrupts globally, including the SoftDevice's interrupts.
 *
 * @retval  The value of PRIMASK prior to disabling the interrupts.
 */
int __sd_nvic_irq_disable(void)
{
  int pm = __get_PRIMASK();
  __disable_irq();
  return pm;
}

/**@brief Enables IRQ interrupts globally, including the SoftDevice's interrupts.
 */
void __sd_nvic_irq_enable(void)
{
  __enable_irq();
}

/**@brief Checks if IRQn is available to application
 * @param[in]  IRQn  irq to check
 *
 * @retval  1 (true) if the irq to check is available to the application
 */
uint32_t __sd_nvic_app_accessible_irq(IRQn_Type IRQn)
{
  if (IRQn < 32)
  {
    return ((1UL<<IRQn) & __NRF_NVIC_APP_IRQS_0) != 0;
  }
#ifdef NRF52
  else if (IRQn < 64)
  {
    return ((1UL<<(IRQn-32)) & __NRF_NVIC_APP_IRQS_1) != 0;
  }
#endif
  else
  {
    return 1;
  }
}

/**@brief Checks if IRQn is available to application
 * @param[in]  priority  priority to check
 *
 * @retval  1 (true) if the priority to check is available to the application
 */
uint32_t __sd_nvic_is_app_accessible_priority(uint32_t priority)
{
  if(priority >= (1 << __NVIC_PRIO_BITS))
  {
    return 0;
  }
#ifdef NRF51
  if(   priority == 0
     || priority == 2
     )
  {
    return 0;
  }
#endif
#ifdef NRF52
  if(   priority == 0
     || priority == 1
     || priority == 4
     || priority == 5
     )
  {
    return 0;
  }
#endif
  return 1;
}

/**@} */

/**@addtogroup NRF_NVIC_FUNCTIONS SoftDevice NVIC public functions
 * @{ */

/**@brief Enable External Interrupt.
 * @note Corresponds to NVIC_EnableIRQ in CMSIS.
 *
 * @pre IRQn is valid and not reserved by the stack.
 *
 * @param[in] IRQn See the NVIC_EnableIRQ documentation in CMSIS.
 *
 * @retval ::NRF_SUCCESS The interrupt was enabled.
 * @retval ::NRF_ERROR_SOC_NVIC_INTERRUPT_NOT_AVAILABLE The interrupt is not available for the application.
 * @retval ::NRF_ERROR_SOC_NVIC_INTERRUPT_PRIORITY_NOT_ALLOWED The interrupt has a priority not available for the application.
 */
uint32_t sd_nvic_EnableIRQ(IRQn_Type IRQn)
{
  if (!__sd_nvic_app_accessible_irq(IRQn))
  {
    return NRF_ERROR_SOC_NVIC_INTERRUPT_NOT_AVAILABLE;
  }
  if (!__sd_nvic_is_app_accessible_priority(NVIC_GetPriority(IRQn)))
  {
    return NRF_ERROR_SOC_NVIC_INTERRUPT_PRIORITY_NOT_ALLOWED;
  }

  if (nrf_nvic_state.__cr_flag)
  {
    nrf_nvic_state.__irq_masks[(uint32_t)((int32_t)IRQn) >> 5] |= (uint32_t)(1 << ((uint32_t)((int32_t)IRQn) & (uint32_t)0x1F));
  }
  else
  {
    NVIC_EnableIRQ(IRQn);
  }
  return NRF_SUCCESS;
}

/**@brief  Disable External Interrupt.
 * @note Corresponds to NVIC_DisableIRQ in CMSIS.
 *
 * @pre IRQn is valid and not reserved by the stack.
 *
 * @param[in] IRQn See the NVIC_DisableIRQ documentation in CMSIS.
 *
 * @retval ::NRF_SUCCESS The interrupt was disabled.
 * @retval ::NRF_ERROR_SOC_NVIC_INTERRUPT_NOT_AVAILABLE The interrupt is not available for the application.
 */
uint32_t sd_nvic_DisableIRQ(IRQn_Type IRQn)
{
  if (!__sd_nvic_app_accessible_irq(IRQn))
  {
    return NRF_ERROR_SOC_NVIC_INTERRUPT_NOT_AVAILABLE;
  }

  if (nrf_nvic_state.__cr_flag)
  {
    nrf_nvic_state.__irq_masks[(uint32_t)((int32_t)IRQn) >> 5] &= ~(1UL << ((uint32_t)(IRQn) & 0x1F));
  }
  else
  {
    NVIC_DisableIRQ(IRQn);
  }

  return NRF_SUCCESS;
}

/**@brief  Get Pending Interrupt.
 * @note Corresponds to NVIC_GetPendingIRQ in CMSIS.
 *
 * @pre IRQn is valid and not reserved by the stack.
 *
 * @param[in]   IRQn          See the NVIC_GetPendingIRQ documentation in CMSIS.
 * @param[out]  p_pending_irq Return value from NVIC_GetPendingIRQ.
 *
 * @retval ::NRF_SUCCESS The interrupt is available for the application.
 * @retval ::NRF_ERROR_SOC_NVIC_INTERRUPT_NOT_AVAILABLE IRQn is not available for the application.
 */
uint32_t sd_nvic_GetPendingIRQ(IRQn_Type IRQn, uint32_t * p_pending_irq)
{
  if (__sd_nvic_app_accessible_irq(IRQn))
  {
    *p_pending_irq = NVIC_GetPendingIRQ(IRQn);
    return NRF_SUCCESS;
  }
  else
  {
    return NRF_ERROR_SOC_NVIC_INTERRUPT_NOT_AVAILABLE;
  }
}

/**@brief  Set Pending Interrupt.
 * @note Corresponds to NVIC_SetPendingIRQ in CMSIS.
 *
 * @pre IRQn is valid and not reserved by the stack.
 *
 * @param[in] IRQn See the NVIC_SetPendingIRQ documentation in CMSIS.
 *
 * @retval ::NRF_SUCCESS The interrupt is set pending.
 * @retval ::NRF_ERROR_SOC_NVIC_INTERRUPT_NOT_AVAILABLE IRQn is not available for the application.
 */
uint32_t sd_nvic_SetPendingIRQ(IRQn_Type IRQn)
{
  if (__sd_nvic_app_accessible_irq(IRQn))
  {
    NVIC_SetPendingIRQ(IRQn);
    return NRF_SUCCESS;
  }
  else
  {
    return NRF_ERROR_SOC_NVIC_INTERRUPT_NOT_AVAILABLE;
  }
}

/**@brief  Clear Pending Interrupt.
 * @note Corresponds to NVIC_ClearPendingIRQ in CMSIS.
 *
 * @pre IRQn is valid and not reserved by the stack.
 *
 * @param[in] IRQn See the NVIC_ClearPendingIRQ documentation in CMSIS.
 *
 * @retval ::NRF_SUCCESS The interrupt pending flag is cleared.
 * @retval ::NRF_ERROR_SOC_NVIC_INTERRUPT_NOT_AVAILABLE IRQn is not available for the application.
 */
uint32_t sd_nvic_ClearPendingIRQ(IRQn_Type IRQn)
{
  if (__sd_nvic_app_accessible_irq(IRQn))
  {
    NVIC_ClearPendingIRQ(IRQn);
    return NRF_SUCCESS;
  }
  else
  {
    return NRF_ERROR_SOC_NVIC_INTERRUPT_NOT_AVAILABLE;
  }
}

/**@brief Set Interrupt Priority.
 * @note Corresponds to NVIC_SetPriority in CMSIS.
 *
 * @pre IRQn is valid and not reserved by the stack.
 * @pre Priority is valid and not reserved by the stack.
 *
 * @param[in] IRQn      See the NVIC_SetPriority documentation in CMSIS.
 * @param[in] priority  A valid IRQ priority for use by the application.
 *
 * @retval ::NRF_SUCCESS The interrupt and priority level is available for the application.
 * @retval ::NRF_ERROR_SOC_NVIC_INTERRUPT_NOT_AVAILABLE IRQn is not available for the application.
 * @retval ::NRF_ERROR_SOC_NVIC_INTERRUPT_PRIORITY_NOT_ALLOWED The interrupt priority is not available for the application.
 */
uint32_t sd_nvic_SetPriority(IRQn_Type IRQn, uint32_t priority)
{
  if (!__sd_nvic_app_accessible_irq(IRQn))
  {
    return NRF_ERROR_SOC_NVIC_INTERRUPT_NOT_AVAILABLE;
  }

  if (!__sd_nvic_is_app_accessible_priority(priority))
  {
    return NRF_ERROR_SOC_NVIC_INTERRUPT_PRIORITY_NOT_ALLOWED;
  }

  NVIC_SetPriority(IRQn, (uint32_t)priority);
  return NRF_SUCCESS;
}

/**@brief Get Interrupt Priority.
 * @note Corresponds to NVIC_GetPriority in CMSIS.
 *
 * @pre IRQn is valid and not reserved by the stack.
 *
 * @param[in]  IRQn         See the NVIC_GetPriority documentation in CMSIS.
 * @param[out] p_priority   Return value from NVIC_GetPriority.
 *
 * @retval ::NRF_SUCCESS The interrupt priority is returned in p_priority.
 * @retval ::NRF_ERROR_SOC_NVIC_INTERRUPT_NOT_AVAILABLE - IRQn is not available for the application.
 */
uint32_t sd_nvic_GetPriority(IRQn_Type IRQn, uint32_t * p_priority)
{
  if (__sd_nvic_app_accessible_irq(IRQn))
  {
    *p_priority = (NVIC_GetPriority(IRQn) & 0xFF);
    return NRF_SUCCESS;
  }
  else
  {
    return NRF_ERROR_SOC_NVIC_INTERRUPT_NOT_AVAILABLE;
  }
}

/**@brief System Reset.
 * @note Corresponds to NVIC_SystemReset in CMSIS.
 *
 * @retval ::NRF_ERROR_SOC_NVIC_SHOULD_NOT_RETURN
 */
uint32_t sd_nvic_SystemReset(void)
{
  NVIC_SystemReset();
  return NRF_ERROR_SOC_NVIC_SHOULD_NOT_RETURN;
}

/**@brief Enters critical region.
 *
 * @post Application interrupts will be disabled.
 * @note sd_nvic_critical_region_enter() and ::sd_nvic_critical_region_exit() must be called in matching pairs inside each
 * execution context
 * @sa sd_nvic_critical_region_exit
 *
 * @retval ::NRF_SUCCESS
 */
uint32_t sd_nvic_critical_region_enter(uint8_t * p_is_nested_critical_region)
{
  int was_masked = __sd_nvic_irq_disable();
  if (!nrf_nvic_state.__cr_flag)
  {
    nrf_nvic_state.__cr_flag = 1;
    nrf_nvic_state.__irq_masks[0] = ( NVIC->ICER[0] & __NRF_NVIC_APP_IRQS_0 );
    NVIC->ICER[0] = __NRF_NVIC_APP_IRQS_0;
    #ifdef NRF52
    nrf_nvic_state.__irq_masks[1] = ( NVIC->ICER[1] & __NRF_NVIC_APP_IRQS_1 );
    NVIC->ICER[1] = __NRF_NVIC_APP_IRQS_1;
    #endif
    *p_is_nested_critical_region = 0;
  }
  else
  {
    *p_is_nested_critical_region = 1;
  }
  if (!was_masked)
  {
    __sd_nvic_irq_enable();
  }
  return NRF_SUCCESS;
}

/**@brief Exit critical region.
 *
 * @pre Application has entered a critical region using ::sd_nvic_critical_region_enter.
 * @post If not in a nested critical region, the application interrupts will restored to the state before ::sd_nvic_critical_region_enter was called.
 *
 * @param[in] is_nested_critical_region If this is set to 1, the critical region won't be exited. @sa sd_nvic_critical_region_enter.
 *
 * @retval ::NRF_SUCCESS
 */
uint32_t sd_nvic_critical_region_exit(uint8_t is_nested_critical_region)
{
  if (nrf_nvic_state.__cr_flag && (is_nested_critical_region == 0))
  {
    int was_masked = __sd_nvic_irq_disable();
    NVIC->ISER[0] = nrf_nvic_state.__irq_masks[0];
    #ifdef NRF52
    NVIC->ISER[1] = nrf_nvic_state.__irq_masks[1];
    #endif
    nrf_nvic_state.__cr_flag = 0;
    if (!was_masked)
    {
      __sd_nvic_irq_enable();
    }
  }

  return NRF_SUCCESS;
}