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

// UART Pin definitions
#define XL1		                      0u
#define XL2		                      1u
#define GPIO_EXT0                   3u
#define GPIO_EXT1                   4u
#define LOGGER_UART_TX              GPIO_EXT0
#define LOGGER_UART_RX              GPIO_EXT1
#define MPU_INT 		                5u
#define MPU_SCL  		                6u
#define MPU_SDA  		                7u
#define BUTTON0                     8u
#define LED0                        10u
#define LED1                        11u
#define LED2                        12u
#define LORA_AUX_PIN_NUMBER         14u
#define LORA_TX_PIN_NUMBER          15u
#define LORA_RX_PIN_NUMBER          16u
#define LORA_M1_PIN_NUMBER          17u
#define LORA_M0_PIN_NUMBER          18u
#define SPI_FLASH_CS                21u
#define SPI_FLASH_MISO              22u
#define SPI_FLASH_WP		            23u
#define SPI_FLASH_SCLK              24u
#define SPI_FLASH_MOSI              25u
#define SPI_FLASH_HOLD	            29u

#define TX_PIN_NUMBER               GPIO_EXT0
#define RX_PIN_NUMBER               GPIO_EXT1
#define CTS_PIN_NUMBER              0u
#define RTS_PIN_NUMBER              0u
#define UART_HWFC                   false

#ifdef __cplusplus
}
#endif
#endif /* ___BSP_PINMAP_H */
