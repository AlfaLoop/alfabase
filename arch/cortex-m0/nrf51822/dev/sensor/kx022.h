 /**
 *  The MIT License (MIT)
 *
 *  Copyright (c) 2014-2015 Ming-Yu Lin <menaya0506@gmail.com>
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
#ifndef __KX022_H__
#define __KX022_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define KX022_XOUTL							0x06
#define KX022_XOUTH 						0x07
#define KX022_WHO_AM_I                      0x0F
#define KX022_CNTL1							0x18
#define KX022_CNTL2							0x19
#define KX022_ODCNTL						0x1B
#define KX022_INC4							0x1F

#define KX022_ACCEL_SCALE_2G				0x00
#define KX022_ACCEL_SCALE_4G				0x08
#define KX022_ACCEL_SCALE_8G				0x10

#define KX022_ACCEL_SAMPLING_RATE_12_5HZ	0x00
#define KX022_ACCEL_SAMPLING_RATE_25HZ		0x01
#define KX022_ACCEL_SAMPLING_RATE_50HZ		0x02
#define KX022_ACCEL_SAMPLING_RATE_100HZ		0x03
#define KX022_ACCEL_SAMPLING_RATE_200HZ		0x04
#define KX022_ACCEL_SAMPLING_RATE_400HZ		0x05

#define KX022_CNTL2_SRST					0x80
#define KX022_INC1_IEN						0x20
#define KX022_INC1_IEA						0x10
#define KX022_INC1_IEL						0x08
#define KX022_INC4_DRDYI 					0x10
#define KX022_ODCNTL_IIR_BYPASS 			0x80
#define KX022_ODCNTL_LPRO					0x40
#define KX022_CNTL1_PC1						0x80
#define KX022_CNTL1_RES						0x40
#define KX022_CNTL1_DRDYE					0x20


uint8_t kx022_init(uint8_t fsr, uint8_t rate);

uint8_t kx022_spi_init();
uint8_t kx022_sensor_init(uint8_t fsr, uint8_t rate);
uint8_t kx022_sensor_disable();

/* Configiration */
uint8_t kx022_sensor_set_accel_fsr(uint8_t fsr);
uint8_t kx022_sensor_set_sample_rate(uint16_t rate);

/* Get Configiration*/
uint8_t kx022_sensor_get_accel_fsr(uint8_t *fsr);
uint8_t kx022_sensor_get_sample_rate(uint16_t *rate);

uint8_t kx022_sensor_get_accel(uint8_t *rx_data);
uint8_t kx022_who_am_i(uint8_t *rx_data);

#ifdef __cplusplus
}
#endif

#endif

