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

Three FreeRTOS Task are used:
1. Highest priority task


## Characteristics

* Supporting tickless scheduler. (event-driven)
* Highest priority task manage all of the interrupt events, hardware resources
  and user application events.   
* Can be dynamically reprogrammed by using ELF loader without kernel reinstall.
* The lowest priority task is used to hook user application. Therefore, User thread
  infinite loop doesn’t hang system.
* The second lowest priority task is used to simulate interrupt (callback) for
  user application.
* Replace coffee file system (The original Contiki's file system) with SPIFFS
  for supporting wear-leveled.
* Uniform Framework API layer, make the user's program can be more lightweight.
* User application build tools, help to generate loadable program and can easy
  re-install to filesystem by using shell.

## Software Prerequisites

The following tools are required to run or debug the firmware:

Tool        | Recommended Version   | Link/Install
---         | ---                   | ---
SoftDevice  | S132-v3 3.1.0         | [Nordic Download](http://www.nordicsemi.com/eng/nordic/Products/nRF52832/S132-SD-v4/58803)
JLink       | v5.12                 | [JLink Download](https://www.segger.com/downloads/jlink)
arm-none-eabi-gcc | 5.4 (2016q2) | [arm-none-eabi-gcc Download](https://www.segger.com/downloads/jlink)


## TODO

* Copy the Makefile.config to Makefile.env
* Setup your compile en

* Dynamically allocate memory from a shared global heap when load and link application.
* Fault isolation (rely on careful API design or memory guard regions)

## Applications

* fitness tracker
* sensor network
*

## GETTING STARTED


## LICENSE
