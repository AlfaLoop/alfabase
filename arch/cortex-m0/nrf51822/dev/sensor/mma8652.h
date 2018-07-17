
#ifndef __MMA8652_H__
#define __MMA8652_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MMA8652_SLAVE_ADDRESS				0x1D << 1
#define MMA8652_DEVICE_ID					0x4A

#define MMA8652_OUT_X_MSB					0x01
#define MMA8652_OUT_X_LSB					0x02
#define MMA8652_OUT_Y_MSB					0x03
#define MMA8652_OUT_Y_LSB					0x04
#define MMA8652_OUT_Z_MSB					0x05
#define MMA8652_OUT_Z_LSB					0x06
#define MMA8652_WHO_AM_I					0x0D
#define MMA8652_XYZ_DATA_CFG				0x0E
#define MMA8652_CTRL_REG1					0x2A
#define MMA8652_CTRL_REG2					0x2B
#define MMA8652_CTRL_REG3					0x2C
#define MMA8652_CTRL_REG4					0x2D
#define MMA8652_CTRL_REG5					0x2E

#define MMA8652_ACCEL_SCALE_2G				0x00
#define MMA8652_ACCEL_SCALE_4G				0x01
#define MMA8652_ACCEL_SCALE_8G				0x02

#define MMA8652_ACCEL_SAMPLING_RATE_12_5HZ	0x28
#define MMA8652_ACCEL_SAMPLING_RATE_25HZ	0x00
#define MMA8652_ACCEL_SAMPLING_RATE_50HZ	0x20
#define MMA8652_ACCEL_SAMPLING_RATE_100HZ	0x18
#define MMA8652_ACCEL_SAMPLING_RATE_200HZ	0x10
#define MMA8652_ACCEL_SAMPLING_RATE_400HZ	0x08

#define MMA8652_CTRL_REG1_STANDBY_MODE		0x00
#define MMA8652_CTRL_REG1_ACTIVE_MODE		0x01
#define MMA8652_CTRL_REG1_DEFAULT			0x00
#define MMA8652_CTRL_REG2_RESET				0x40
#define MMA8652_CTRL_REG2_DEFAULT			0x00
#define MMA8652_CTRL_REG3_DEFAULT			0x00
#define MMA8652_CTRL_REG4_DEFAULT			0x00
#define MMA8652_CTRL_REG5_DEFAULT			0x00

uint8_t mma8652_init(uint8_t fsr, uint8_t rate);
uint8_t mma8652_sensor_disable();

uint8_t mma8652_sensor_set_accel_fsr(uint8_t fsr);
uint8_t mma8652_sensor_set_sample_rate(uint8_t rate);
uint8_t mma8652_sensor_get_accel_fsr(uint8_t *fsr);
uint8_t mma8652_sensor_get_sample_rate(uint8_t *rate);

uint8_t mma8652_standby(void);
uint8_t mma8652_active(void);

uint8_t mma8652_sensor_get_accel(uint8_t *rx_data);
uint8_t mma8652_who_am_i(uint8_t *rx_data);

#define ERROR_CODE_CHECK( er )            \
		if(er > 0)                        \
		{                                 \
			return er;                    \
		}

#ifdef __cplusplus
}
#endif

#endif

