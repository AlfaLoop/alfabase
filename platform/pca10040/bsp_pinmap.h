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
#define ARDUINO_AREF_PIN            2     // Aref pin
#define ARDUINO_A0_PIN              3     // Analog channel 0
#define ARDUINO_A1_PIN              4     // Analog channel 1
#define RTS_PIN_NUMBER              5
#define TX_PIN_NUMBER               6
#define CTS_PIN_NUMBER              7
#define RX_PIN_NUMBER               8
#define UART_HWFC                   false
#define ARDUINO_0_PIN               11    // Digital pin 0
#define ARDUINO_1_PIN               12    // Digital pin 1
#define LOGGER_UART_TX              TX_PIN_NUMBER
#define LOGGER_UART_RX              RX_PIN_NUMBER

#define BUTTON0                     13
#define BUTTON1                     14
#define BUTTON2                     15
#define BUTTON3                     16
#define LED0                        17
#define LED1                        18
#define LED2                        19
#define LED3                        20

// Arduino board mappings
#define ARDUINO_10_PIN              22    // Digital pin 10
#define ARDUINO_11_PIN              23    // Digital pin 11
#define ARDUINO_12_PIN              24    // Digital pin 12
#define ARDUINO_13_PIN              25    // Digital pin 13
#define ARDUINO_SDA_PIN             26    // SDA signal pin
#define ARDUINO_SCL_PIN             27    // SCL signal pin
#define ARDUINO_A2_PIN              28    // Analog channel 2
#define ARDUINO_A3_PIN              29    // Analog channel 3
#define ARDUINO_A4_PIN              30    // Analog channel 4
#define ARDUINO_A5_PIN              31    // Analog channel 5

#ifdef __cplusplus
}
#endif
#endif /* ___BSP_PINMAP_H */
