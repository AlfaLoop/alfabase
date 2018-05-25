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
#define BUTTON0                  2u
#define BUZZER_PIN               3u
#define LED0                     4u
#define SPI_FLASH_CS             6u
#define SPI_FLASH_SCLK           7u
#define SPI_FLASH_MISO           5u
#define SPI_FLASH_MOSI           8u
#define LED1                     11u
#define GPIO_EXT0                12u
#define GPIO_EXT1                13u
#define GPIO_EXT2                14u
#define GPIO_EXT3                15u
#define RADIO_PIN_0              16u
#define RADIO_PIN_1              17u
#define RADIO_PIN_2              18u
#define RADIO_PIN_3              19u
#define RADIO_PIN_4              20u
#define MPU_SDA  		             25u
#define MPU_SCL  		             26u
#define MPU_CS  		             27u
#define MPU_INT 		             28u
#define MPU_AUX  		             29u
#define BUTTON1                  30u
#define LED2                     31u

#define RTS_PIN_NUMBER           GPIO_EXT2
#define TX_PIN_NUMBER            GPIO_EXT0
#define CTS_PIN_NUMBER           GPIO_EXT3
#define RX_PIN_NUMBER            GPIO_EXT1
#define UART_HWFC                false
#define LOGGER_UART_TX           TX_PIN_NUMBER
#define LOGGER_UART_RX           RX_PIN_NUMBER

#ifdef __cplusplus
}
#endif
#endif /* ___BSP_PINMAP_H */
