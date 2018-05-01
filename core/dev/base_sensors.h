/**
 *  Copyright (c) 2016 AlfaLoop Technology Co., Ltd. All Rights Reserved.
 *
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential.
 *
 *  Attribution - You must give appropriate credit, provide a link to the license, and
 *  indicate if changes were made. You may do so in any reasonable manner, but not in any
 *  way that suggests the licensor endorses you or your use.
 *
 *  NonCommercial - You may not use the material for commercial purposes under unauthorized.
 *
 *  NoDerivatives - If you remix, transform, or build upon the material, you may not
 *  distribute the modified material.
 */
#ifndef _BASE_SENSORS_H_
#define _BASE_SENSORS_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	accelerometer_type = 0x00,
	temperture_type,
	magnetic_type,
	gyroscope_type,
	heartrate_type,
	light_type,
	battery_type,
	proximity_type,
	pressure_type,
	humidity_type,
	axis9fuse_type,
	axis6fuse_type
} base_sensors_type_t;

typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} accel_sensor_data_t;

typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} gyro_sensor_data_t;

typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} magnetic_sensor_data_t;

typedef struct {
	int16_t value;
} humidity_sensor_data_t;

typedef struct {
	int16_t value;
} temperture_sensor_data_t;

typedef struct {
	int16_t value;
} light_sensor_data_t;

typedef struct {
	int16_t hrps;
	int16_t rri;
} heartrate_sensor_data_t;

typedef union
{
	accel_sensor_data_t 		accel_data;
	gyro_sensor_data_t			gyro_data;
	magnetic_sensor_data_t  	magnetic_data;
	humidity_sensor_data_t		humidity_data;
	temperture_sensor_data_t 	temperture_data;
	light_sensor_data_t			light_data;
	heartrate_sensor_data_t 	heartrate_data;
} sensor_share_data_t;

typedef struct {
	void *data;
	uint32_t 	timestamp;
} sensor_data_t;

typedef struct {
	char 	*name;
	uint8_t  opcode;
} sensor_opcode_t;


#ifdef __cplusplus
}
#endif
#endif /* _BASE_SENSORS_H_ */
