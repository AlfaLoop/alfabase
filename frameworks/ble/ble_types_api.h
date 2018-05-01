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
#ifndef _BLE_TYPES_API_H
#define _BLE_TYPES_API_H
#ifdef __cplusplus
extern "C" {
#endif


#define BLE_GATT_MAX_CHARATERISTICS_NUM   5

#define ADVERTISE_MAX_DATA_SIZE           31				// Maximum size of advertising data in octets.

#define ADVERTISE_MODE_SUPER_FAST         0x05           /* 76.25 ms */
#define ADVERTISE_MODE_FAST               0x04           /* 150  ms */
#define ADVERTISE_MODE_LOW_LATENCY        0x03           /* 200  ms */
#define ADVERTISE_MODE_BALANCED           0x02           /* 500  ms */
#define ADVERTISE_MODE_LOW_POWER          0x01           /* 1000  ms */
#define ADVERTISE_MODE_ULTRA_LOW_POWER    0x00           /* 4000  ms */

#define ADVERTISE_TX_POWER_HIGH         0x03
#define ADVERTISE_TX_POWER_MEDIUM       0x02
#define ADVERTISE_TX_POWER_LOW          0x01
#define ADVERTISE_TX_POWER_ULTRA_LOW    0x00

#define ADVERTISE_CALLBACK_STATUS_TIMEOUT      0x01

#define ADVERTISE_INTERVAL_MIN_MS   	 20	    /** < Minimum Advertising interval in 625 us units, i.e. 20 ms. */
#define ADVERTISE_INTERVAL_MAX_MS   	 10240	/** < Maximum Advertising interval in 625 us units, i.e. 10.24 s. */

#define BLE_UUID_TYPE_SIG_BLE       0x01  // Bluetooth SIG UUID (16-bit).
#define BLE_UUID_TYPE_VENDOR        0x02  // Vendor UUID types start at this index (128-bit).

#ifdef __cplusplus
}
#endif
#endif /* _BLE_TYPES_API_H */
