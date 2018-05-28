# AlfaBeacon


## AlfaBeacon Frame

AlfaBeacon is made up of the following elements

* Apple iBeacon packets
* AlfaBeacon Service uuid:  0xA55A
* AlfaBeacon Service data:

Byte<br/> Offset | Description<br/> | Example <br/> Value
--:|:------:|:------:|  
0 | Device type | 0:(AlfaAA) 1:(AlfaUSB) 2:(Alfa2477) 3:(Alfa2477s) |
 1 | Battery level | (0x00~0x64) (0~100)|
 2~7 | MAC Address| AABBCCDDEEFF |
 
 
* Device name: AlfaBeacon (scan response)
