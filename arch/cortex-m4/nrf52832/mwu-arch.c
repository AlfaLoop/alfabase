/* Copyright (c) 2016, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   * Neither the name of Nordic Semiconductor ASA nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
