/**
 *  The MIT License (MIT)
 *
 *  Copyright (c) 2014-2015 Chun-Ting Ding <jiunting.d@gmail.com>
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#include "contiki.h"
#include "mma8652.h"
#include "dev/i2c.h"

#if defined(DEBUG_ENABLE)
#define DEBUG_MODULE 1
#if DEBUG_MODULE
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif  /* DEBUG_MODULE */
#else
#define PRINTF(...)
#endif  /* DEBUG_ENABLE */

static uint8_t mma8652_read(uint8_t address, uint8_t reg, uint32_t length, uint8_t * p_data) 
{
	int err_code;

	err_code = i2c_operate(I2C_TX, address, 1, &reg, I2C_DONT_ISSUE_STOP);
	
    if (err_code) {
		return err_code;
	}

    err_code = i2c_operate(I2C_RX, address, length, p_data, I2C_ISSUE_STOP);
	return err_code;
}

static uint8_t mma8652_read_byte(uint8_t address, uint8_t reg, uint8_t * p_data) 
{
	int err_code;
	err_code = i2c_operate(I2C_TX, address, 1, &reg, I2C_DONT_ISSUE_STOP);
    if (err_code) {
		return err_code;
	}
    err_code = i2c_operate(I2C_RX, address, 1, p_data, I2C_ISSUE_STOP);
	return err_code;
}

static uint8_t mma8652_write_byte(uint8_t address, uint8_t reg, uint8_t data) 
{
	int err_code;
	uint8_t tx[2] = {reg, data};
	err_code = i2c_operate(I2C_TX, address, 2, tx, I2C_ISSUE_STOP);
	return err_code;
}

static uint8_t mma8652_i2c_init()
{
    return i2c_init();
}

static uint8_t mma8652_sensor_init(uint8_t fsr, uint8_t rate)
{
	uint32_t err_code;

	err_code = mma8652_sensor_set_accel_fsr(fsr);
	ERROR_CODE_CHECK(err_code);

	err_code = mma8652_sensor_set_sample_rate(rate);
	ERROR_CODE_CHECK(err_code);
	
	return err_code;
}

uint8_t mma8652_init(uint8_t fsr, uint8_t rate)
{
	uint32_t err_code;
	
	err_code = mma8652_i2c_init();
	
	err_code = mma8652_sensor_init(fsr, rate);
	
	return err_code;
}

uint8_t mma8652_sensor_disable()
{
	return i2c_disable();
}

uint8_t mma8652_active_interrupt(void)
{
	uint8_t value = 0;
	/*
	if(mma8652_read(MMA8652_SLAVE_ADDRESS, MMA8652_CTRL_REG3, 1, &value) > 0) {
		PRINTF("[Debug] MMA8652_CTRL_REG1 read fail\n");
		return false;
	}*/
	
	uint8_t er = 0;
	
	er=mma8652_write_byte(MMA8652_SLAVE_ADDRESS, MMA8652_CTRL_REG3, value | 2);
	PRINTF("[Debug] er1:%i\n", er);
	er=mma8652_write_byte(MMA8652_SLAVE_ADDRESS, MMA8652_CTRL_REG4, value | 1);
	PRINTF("[Debug] er2:%i\n", er);
	er = mma8652_write_byte(MMA8652_SLAVE_ADDRESS, MMA8652_CTRL_REG5, value | 1);
	PRINTF("[Debug] er3:%i\n", er);
	
	return er;
}

uint8_t mma8652_sensor_set_accel_fsr(uint8_t fsr)
{
	return mma8652_write_byte(MMA8652_SLAVE_ADDRESS, MMA8652_XYZ_DATA_CFG, fsr);
}

uint8_t mma8652_sensor_set_sample_rate(uint8_t rate)
{
	return mma8652_write_byte(MMA8652_SLAVE_ADDRESS, MMA8652_CTRL_REG1, rate);
}

uint8_t mma8652_standby(void)
{
	uint8_t value = 0;
	
	if(mma8652_read(MMA8652_SLAVE_ADDRESS, MMA8652_CTRL_REG1, 1, &value) > 0) {
		PRINTF("[Debug] MMA8652_CTRL_REG1 read fail\n");
		return false;
	}
	
	return mma8652_write_byte(MMA8652_SLAVE_ADDRESS, MMA8652_CTRL_REG1, value & ~(MMA8652_CTRL_REG1_STANDBY_MODE));
}

uint8_t mma8652_active(void)
{
	uint8_t value = 0;
	
	if(mma8652_read(MMA8652_SLAVE_ADDRESS, MMA8652_CTRL_REG1, 1, &value) > 0) {
		PRINTF("[Debug] MMA8652_CTRL_REG1 read fail\n");
		return false;
	}
	
	return mma8652_write_byte(MMA8652_SLAVE_ADDRESS, MMA8652_CTRL_REG1, value | MMA8652_CTRL_REG1_ACTIVE_MODE);
}

uint8_t mma8652_sensor_get_accel_fsr(uint8_t *fsr)
{
	uint32_t err_code;
	
	err_code = mma8652_read(MMA8652_SLAVE_ADDRESS, MMA8652_XYZ_DATA_CFG, 1, fsr);
	
	return err_code;
}

uint8_t mma8652_sensor_get_sample_rate(uint8_t *rate)
{
	uint32_t err_code;
	
	err_code = mma8652_read(MMA8652_SLAVE_ADDRESS, MMA8652_CTRL_REG1, 1, rate);
	
	return err_code;
}

uint8_t mma8652_sensor_get_accel(uint8_t *rx_data)
{
	return mma8652_read(MMA8652_SLAVE_ADDRESS, MMA8652_OUT_X_MSB, 6, rx_data);
}

uint8_t mma8652_who_am_i(uint8_t *rx_data)
{	
	return mma8652_read(MMA8652_SLAVE_ADDRESS, MMA8652_WHO_AM_I, 1, rx_data);
}


