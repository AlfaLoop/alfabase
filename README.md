Alfabase
================

Alfabase is an embedded software framework targeted for IoT/Wearable applications
for resource constrained devices. Alfabase's system is an integration
of FreeRTOS with Contiki OS. The aims of this project are:
* Support application reprogramming(OTA) without updating the whole system. (ELF Loader)
* Provide the framework layer (API) for rapid and easy development of IoT/Wearable applications.
* Infinite loop in an application doesn’t hang system. (Preemptive)
* The system can be remotely managed by using Bluetooth Low Energy or Serial/Uart interface.
* Simply deploy and compile an ELF file from the application (C source) by using tools/alfa.py tools.

## Architecture

TODO

## Characteristics

* Supporting tickless scheduler. (event-driven)
* Replace coffee file system (The original Contiki's file system) with SPIFFS
  for supporting wear-leveled.
* TODO

## Software Prerequisites

The following tools are required to run or debug the firmware:

Tool        | Recommended Version   | Link/Install
---         | ---                   | ---
SoftDevice  | S132-v3 3.1.0         | [Nordic Download](https://www.nordicsemi.com/eng/nordic/Products/nRF52832/S132-SD-v3/56261)
JLink       | v5.12                 | [JLink Download](https://www.segger.com/downloads/jlink)
arm-none-eabi-gcc | 5.4 (2016q2) | [arm-none-eabi-gcc Download](https://launchpad.net/gcc-arm-embedded)

## GETTING STARTED

* Copy the Makefile.config to Makefile.env
* TODO

## LICENSE

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
