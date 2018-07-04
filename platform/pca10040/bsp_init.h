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
#ifndef ___BSP_INIT_H
#define ___BSP_INIT_H
#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_sdm.h"

/** Specify the platform identification */
#define PLATFORM_DEVICE_NAME	                  "PCA10040"
#define PLATFORM_DEVICE_TX_POWER                0

/** Platform-dependent definitions */
#define CC_CONF_REGISTER_ARGS                    0
#define CC_CONF_FUNCTION_POINTER_ARGS            1
#define CC_CONF_FASTCALL
#define CC_CONF_VA_ARGS                          1
#define CC_CONF_INLINE                           inline
#define SD_BLE_GATTS_ATTR_TAB_SIZE_CONF				   0x600
#define SD_CLOCK_SOURCE_CONF                     false      // (false:internal true:Ext)
#define SD_BLE_MAX_MTU_SIZE_CONF					       23
#define GAP_CONF_MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)          /**< Minimum acceptable connection interval (0.04 seconds). */
#define GAP_CONF_MAX_CONN_INTERVAL               MSEC_TO_UNITS(60, UNIT_1_25_MS)          /**< Maximum acceptable connection interval (1 second). */
#define GAP_CONF_SLAVE_LATENCY                   0                                        /**< Slave latency. */
#define GAP_CONF_CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)          /**< Connection supervisory timeout (4 seconds). */

/** Nest communication stack configuration */
#define NEST_CONF_DRIVER				                nrf_nest_driver
#define NEST_PLATFORM_VERSION_H_CONF            0x01
#define NEST_PLATFORM_VERSION_L_CONF            0x01
#define NEST_ADV_TYPE_NFD_RSSI_CONF            -44
#define NEST_ADD_GATTS_SERVICE_CONF             5
#define NEST_ADD_GATTS_CHARACTERISTIC_CONF      20
#define NEST_CENTRAL_USER_LINK_COUNT_CONF       2
#define NEST_PERIPHERAL_LINK_COUNT_CONF         1
#define NEST_CONF_COMMAND_RXQ_ITEMS			        10
#define NEST_CONF_SCANNER_RXQ_ITEMS             8
#define NEST_CHANNEL_CENTRAL_CONF               1
#define NEST_CHANNEL_SERIAL_CONF                0
#define NEST_COMMAND_ENABLE_CONF                1
#define NEST_COMMAND_BFTP_ENABLE_CONF           1
#define NEST_COMMAND_CORE_ENABLE_CONF           1
#define NEST_COMMAND_LUNCHR_ENABLE_CONF         1
#define NEST_COMMAND_AUTH_ENABLE_CONF           1
#define NEST_COMMAND_PIPE_ENABLE_CONF           1
#define NEST_SERIAL_LINE_CONF_BUFSIZE           256

/** FreeRTOS configuration */
#define LUNCHR_KERNEL_USER_APP_IRQ_TASK_SIZE      384
#define LUNCHR_KERNEL_USER_APP_TASK_SIZE		      384
#define CONTIKI_KERNEL_TASK_SIZE				          512

#define FREERTOS_TOTAL_HEAP_SIZE_CONF           ( ( size_t ) (8 * 1024) + 256)
#define FREERTOS_MAX_PRIORITY_CONF              6
#define FREERTOS_TIMER_TASK_PRIORITY_CONF       FREERTOS_MAX_PRIORITY_CONF - 1  // (5)
#define FREERTOS_CONTIKI_TASK_PRIORITY_CONF     FREERTOS_MAX_PRIORITY_CONF - 2  // (4)
#define FREERTOS_APP_TASK_PRIORITY_CONF         1
#define FREERTOS_APP_IRQ_TASK_PRIORITY_CONF     2

/** ELF loader data and memory size */
#define CONTIKI_ELFLOADER_DATAMEMORY_SIZE_CONF  0x5000
#define CONTIKI_ELFLOADER_TEXTMEMORY_SIZE_CONF  ELF_LOADER_TEXT

// Crypto stack configuration
#define CRYPTO_AES_ECB_CONF                       1
#define CRYPTO_AES_ECB_ENCRYPT_HW_CONF            1
#define CRYPTO_AES_ECB_DECRYPT_HW_CONF            0
#define CRYPTO_SHA256_CONF                        1
#define CRYPTO_HMAC_SHA256_CONF                   1

// Framework API usage configuration
#define FRAMEWORK_USE_HEALTHKIT_CONF                0
#define FRAMEWORK_USE_SENSOR_ACCELEROMETE_CONF      0
#define FRAMEWORK_USE_SENSOR_MOTION_RAW_CONF        0
#define FRAMEWORK_USE_SENSOR_MOTION_FUSION_CONF     0
#define FRAMEWORK_USE_SENSOR_HEARTRATE_CONF         0
#define FRAMEWORK_USE_SENSOR_FOOT_PRESSURE_CONF     0

#define PM_CONF_DRIVER                      pm_bsp_driver
#define ARCH_BATTERY_ADC_VENDOR_CONF        "Not defined"
#define ARCH_BATTERY_ADC_CHANNEL_CONF       0

#define WDUI_DRIVER_CONF				         wdui_ssd1316_driver
#define WDUI_MLCD_DRIVER_CONF            spi1_driver
#define WDUI_SCREEN_CLOCKFACE_CONF         0
#define WDUI_SCREEN_STEPS_CONF             0
#define WDUI_SCREEN_BURNED_CONF            0
#define WDUI_SCREEN_DISTANCE_CONF          0
#define WDUI_SCREEN_HRM_CONF               0
#define WDUI_SCREEN_IOTA_CONF              0
#define WDUI_MLCD_FONT_DRIVER_CONF         spi2_driver

//#define ATCMD_CONF_DRIVER                 atcmd_bsp_driver
//#define ATCMD_LINE_CONF_BUFSIZE           256

extern const struct adc_driver           nrf_adc_arch_driver;
#define ADC                              nrf_adc_arch_driver

#define USE_SPIFFS_CACHE                            1
#define STORAGE_SYSC_INTERNAL_CONF                  1
#if STORAGE_SYSC_INTERNAL_CONF == 1
  #define STORAGE_SYSC_FS_INSTANCE_CONF nrf_spiffs
#elif STORAGE_SYSC_INTERNAL_CONF == 2
  #define STORAGE_SYSC_FS_INSTANCE_CONF extfs
  #include "spiffs-flash-arch.h"
  #define STORAGE_SYSC_FS_FLSAH_SIZE_CONF          W25Q20_SIZE
  #define STORAGE_SYSC_FS_FLSAH_PAGES_CONF         W25Q20_PAGES
  #define STORAGE_SYSC_FS_FLSAH_PAGE_SIZE_CONF     W25Q20_PAGE_SIZE
  #define STORAGE_SYSC_FS_FLSAH_SECTOR_SIZE_CONF   W25Q20_SECTOR_SIZE
#endif

/** System log configuration */
#define SYSLOG_CONF_FLOAT			                  true
#if defined(DEBUG_ENABLE)
#define DEBUG_ENABLE true
#else defined(DEBUG_DISABLE)
#undef DEBUG_ENABLE
#endif

#include "bsp_pinmap.h"

#ifdef __cplusplus
}
#endif
#endif /* ___BSP_INIT_H */
