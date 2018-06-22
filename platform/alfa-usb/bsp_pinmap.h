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
#ifndef ___BSP_PINMAP_H
#define ___BSP_PINMAP_H
#ifdef __cplusplus
extern "C" {
#endif

// Board Defintion
/*---------------------------------------------------------------------------*/
#define XL1		                   0u
#define XL2		                   1u
#define LED0                     27u
#define LED1                     28u
#define RX_PIN_NUMBER            22u
#define TX_PIN_NUMBER            23u
#define RTS_PIN_NUMBER           24u
#define CTS_PIN_NUMBER           25u
#define UART_HWFC                false
#define LOGGER_UART_TX           TX_PIN_NUMBER
#define LOGGER_UART_RX           RX_PIN_NUMBER

#ifdef __cplusplus
}
#endif
#endif /* ___BSP_PINMAP_H */
