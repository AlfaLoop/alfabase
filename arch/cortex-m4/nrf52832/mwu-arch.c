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
#include "nrf.h"
#include "bsp_init.h"
#include "errno.h"
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
// Six memory regions, four user-configurable and two fixed regions in peripheral address space
#define UART_ADDRESS 0x40002000
#define UART_ADDRESSEND 0x40003000
#define NRF_UARTE0                      ((NRF_UARTE_Type          *) NRF_UARTE0_BASE)
/*---------------------------------------------------------------------------*/
void
MWU_IRQHandler( void )
{
  PRINTF("[mpu-arch] MWU_IRQHandler\n");
}
/*---------------------------------------------------------------------------*/
void
mpu_arch_init(void)
{
  NRF_MWU->REGION[3].START = UART_ADDRESS;
  NRF_MWU->REGION[3].END   = UART_ADDRESSEND;
  NRF_MWU->INTENSET    = MWU_INTENSET_REGION3WA_Enabled << MWU_INTENSET_REGION3WA_Pos;
  NRF_MWU->REGIONENSET = MWU_REGIONENSET_RGN3WA_Enabled << MWU_REGIONENSET_RGN3WA_Pos;
  sd_nvic_SetPriority( MWU_IRQn, 7 );
  sd_nvic_ClearPendingIRQ( MWU_IRQn );
  sd_nvic_EnableIRQ( MWU_IRQn );
  PRINTF("[mpu-arch] mpu_arch_init\n");
}
/*---------------------------------------------------------------------------*/
