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
#ifndef __ALFA2477s_PROFILE_H_
#define __ALFA2477s_PROFILE_H_
#ifdef __cplusplus
extern "C" {
#endif

// Alfa2477s Beacon Service Profile
/* {903E0001-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid128 gatt_alfa_2477s_service_uuid =
    BLE_UUID128_INIT(0x97, 0x12, 0xa3, 0xbc, 0x8a, 0x12, 0xF9, 0xE9,
                     0xa3, 0x09, 0xc2, 0xF6, 0x01, 0x00, 0x3E, 0x90);
/* Alfa2477s RF attenuator UUID {903E0002-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfa_2477s_chr_rfatte = BLE_UUID16_INIT(0x0002);

/* Alfa2477s Button UUID {903E0003-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfa_2477s_chr_button = BLE_UUID16_INIT(0x0003);

/* Alfa2477s Buzzer UUID {903E0004-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfa_2477s_chr_buzzer = BLE_UUID16_INIT(0x0004);

/* Alfa2477s Led UUID {903E0005-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfa_2477s_chr_led = BLE_UUID16_INIT(0x0005);


#ifdef __cplusplus
}
#endif
#endif /* __ALFA2477s_PROFILE_H_ */
