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

#define BOARD_FOOT_V2_RIGHT            0
#define BOARD_FOOT_V2_LEFT             1
#define BOARD_FOOT_V3_RIGHT            2
#define BOARD_FOOT_V3_LEFT             3

#if defined(BDV)
#define BOARD_DEV_VERSION              BDV
#else
#define BOARD_DEV_VERSION              BOARD_FOOT_V3_LEFT
#endif

// Board Defintion
/*---------------------------------------------------------------------------*/
#if BOARD_DEV_VERSION == BOARD_FOOT_V2_RIGHT
#define ADC_INPUT_PIN_HEEL          2u
#define ADC_INPUT_PIN_OUTTER        3u
#define ADC_INPUT_PIN_THUMB         4u
#define ADC_INPUT_PIN_INNER         5u
#elif BOARD_DEV_VERSION == BOARD_FOOT_V2_LEFT
#define ADC_INPUT_PIN_HEEL          2u
#define ADC_INPUT_PIN_INNER         3u
#define ADC_INPUT_PIN_THUMB         4u
#define ADC_INPUT_PIN_OUTTER        5u
#elif BOARD_DEV_VERSION == BOARD_FOOT_V3_RIGHT
#define ADC_INPUT_PIN_INNER         2u
#define ADC_INPUT_PIN_OUTTER        3u
#define ADC_INPUT_PIN_THUMB         4u
#define ADC_INPUT_PIN_HEEL          31u
#elif BOARD_DEV_VERSION == BOARD_FOOT_V3_LEFT
#define ADC_INPUT_PIN_OUTTER        2u
#define ADC_INPUT_PIN_INNER         3u
#define ADC_INPUT_PIN_THUMB         4u
#define ADC_INPUT_PIN_HEEL          31u
#endif

#if BOARD_DEV_VERSION == BOARD_FOOT_V2_RIGHT || BOARD_DEV_VERSION == BOARD_FOOT_V2_LEFT
#define SPI_FLASH_MISO              12u
#define SPI_FLASH_CS                13u
#define SPI_FLASH_SCLK              14u
#define SPI_FLASH_MOSI              15u
#define MPU_SDA  		                16u
#define MPU_SCL  		                17u
#define MPU_CS  		                18u
#define MPU_INT 		                19u
#define MPU_AUX  		                20u
#define UART_RX                     22u
#define UART_TX                     23u
#define LOGGER_UART_TX              UART_TX
#define LOGGER_UART_RX              UART_RX
#define LED0                        29u
#define BATTERY_ADC_PIN		          31u
#define MPU_ADDRESS				          0xD0
#elif BOARD_DEV_VERSION == BOARD_FOOT_V3_RIGHT || BOARD_DEV_VERSION == BOARD_FOOT_V3_LEFT
#define BATTERY_ADC_PIN		          5u
#define SPI_FLASH_MISO              7u
#define SPI_FLASH_CS                8u
#define SPI_FLASH_SCLK              13u
#define SPI_FLASH_MOSI              14u
#define CHG_LEV_DET                 15u
#define LED0                        18u
#define CHG_STAT_DET	              19u  // IO，input (interrupt), HIGH: battery full or not charging, LOW: charging
#define UART_RX                     22u
#define UART_TX                     23u
#define LOGGER_UART_TX              UART_TX
#define LOGGER_UART_RX              UART_RX
#define MPU_SDA  		                25u
#define MPU_SCL  		                26u
#define MPU_CS  		                27u
#define MPU_AUX  		                28u
#define MPU_INT 		                30u
#define MPU_ADDRESS				          0xD2
#endif

#define TX_PIN_NUMBER               UART_TX
#define RX_PIN_NUMBER               UART_RX
#define CTS_PIN_NUMBER              0u
#define RTS_PIN_NUMBER              0u
#define UART_HWFC                   false


#ifdef __cplusplus
}
#endif
#endif /* ___BSP_PINMAP_H */
