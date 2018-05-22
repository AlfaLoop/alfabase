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
#ifndef __RADIO_PROFILE_H_
#define __RADIO_PROFILE_H_
#ifdef __cplusplus
extern "C" {
#endif

// Alfa2477s Radio Service Profile
/* {19FA0001-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid128 gatt_alfa_radio_service_uuid =
    BLE_UUID128_INIT(0x97, 0x12, 0xa3, 0xbc, 0x8a, 0x12, 0xF9, 0xE9,
                     0xa3, 0x09, 0xc2, 0xF6, 0x01, 0x00, 0xFA, 0x19);
/* Alfa2477s Radio interval UUID {19FA0002-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfa_radio_chr_interval = BLE_UUID16_INIT(0x0002);

/* Alfa2477s Radio tx power UUID {19FA0003-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfa_radio_chr_tx_power = BLE_UUID16_INIT(0x0003);


#ifdef __cplusplus
}
#endif
#endif /* __RADIO_PROFILE_H_ */
