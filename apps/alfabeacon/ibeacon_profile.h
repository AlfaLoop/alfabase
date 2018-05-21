/*
* Copyright (C) 2016 AlfaLoop Technology Co., Ltd.
*
* Licensed under the Apache License, Version 2.0 (the "License";
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* 	  www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef __IBEACON_PROFILE_H_
#define __IBEACON_PROFILE_H_
#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_IBEACON_COMPANY_IDENTIFIER          0x004C                            /**< Company identifier for Nordic Semiconductor ASA. as per www.bluetooth.org. */

#define DEFAULT_IBEACON_BEACON_INFO_LENGTH          0x17                              /**< Total length of information advertised by the Beacon. */
#define DEFAULT_IBEACON_DEVICE_TYPE                 0x02                              /**< 0x02 refers to Beacon. */
#define DEFAULT_IBEACON_ADV_DATA_LENGTH             0x15                              /**< Length of manufacturer specific data in the advertisement. */
#define DEFAULT_IBEACON_UUID            0x15, 0x34, 0x51, 0x64, \
                                        0x67, 0xAB, 0x3E, 0x49, \
                                        0xF9, 0xD6, 0xE2, 0x90, \
                                        0x00, 0x00, 0x00, 0x08            /**< Proprietary UUID for Beacon. */
#define DEFAULT_IBEACON_MAJOR_VALUE                 0x17, 0x74                        /**< Major value used to identify Beacons. */
#define DEFAULT_IBEACON_MINOR_VALUE                 0x00, 0x09                        /**< Minor value used to identify Beacons. */
#define DEFAULT_IBEACON_MEASURED_RSSI               0xC3                              /**< The Beacon's measured RSSI at 1 meter distance in dBm. */

// AlfaBeacon iBeacon Profile
/* {A78E0001-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid128 gatt_alfa_ibeacon_service_uuid =
    BLE_UUID128_INIT(0x97, 0x12, 0xa3, 0xbc, 0x8a, 0x12, 0xF9, 0xE9,
                     0xa3, 0x09, 0xc2, 0xF6, 0x01, 0x00, 0x8E, 0xA7);
/* iBeacon UUID {A78E0002-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfa_ibeacon_chr_uuid = BLE_UUID16_INIT(0x0002);

/* iBeacon Major {A78E0003-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfa_ibeacon_chr_major = BLE_UUID16_INIT(0x0003);

/* iBeacon Minor {A78E0004-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfa_ibeacon_chr_minor = BLE_UUID16_INIT(0x0004);

/* iBeacon Minor {A78E0005-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfa_ibeacon_chr_txm = BLE_UUID16_INIT(0x0005);

#ifdef __cplusplus
}
#endif
#endif /* __IBEACON_PROFILE_H_ */
