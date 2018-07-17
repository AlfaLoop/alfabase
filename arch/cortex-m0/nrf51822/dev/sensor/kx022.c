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
#include "contiki.h"
#include "kx022.h"
#include "dev/spi.h"

static bool is_spi_active = false;

uint8_t kx022_read(uint8_t start_reg, uint8_t *data, uint16_t len) 
{
	uint8_t cmd = start_reg | 0x80;
	return spi_send_and_recv(&cmd, 1, data, len);
}

uint8_t kx022_read_byte(uint8_t reg, uint8_t *data, uint16_t len) 
{
	uint8_t cmd = reg | 0x80;
	return spi_send_and_recv(&cmd, 1, data, 2); 
}

uint8_t kx022_write_byte(uint8_t reg, uint8_t val) 
{
	uint8_t buf[] = {reg, val};
	uint8_t data = 0x00;
	return spi_send_and_recv(buf, 2, &data, 1);
}

/* 
 * init spi and sensor
 */
uint8_t kx022_init(uint8_t fsr, uint8_t rate)
{
	uint32_t err_code = NRF_ERROR_INVALID_STATE;
	
	err_code = kx022_spi_init();
	ERROR_CODE_CHECK(err_code);
	
	if(is_spi_active)
		err_code = kx022_sensor_init(fsr, rate);
	
	return err_code;
}

uint8_t kx022_spi_init()
{
	uint8_t err_code = NRF_SUCCESS;
	
	spi_init_params_t param;
	param.spi_freq  = SPI_FREQUENCY_M1;
	param.spi_order = SPI_MSB;
	param.mode      = SPI_MODE0;
	
	err_code = spi_init(&param);
	
	if(err_code == NRF_SUCCESS)
		is_spi_active = true;
		
    return err_code;
}

uint8_t kx022_sensor_init(uint8_t fsr, uint8_t rate)
{
	uint32_t err_code = NRF_ERROR_INVALID_STATE;
	
	if(is_spi_active)
	{
		err_code = kx022_write_byte(KX022_CNTL2, KX022_CNTL2_SRST);
		ERROR_CODE_CHECK(err_code);
		
		err_code = kx022_write_byte(KX022_CNTL1, 0);
		ERROR_CODE_CHECK(err_code);
		
		err_code = kx022_write_byte(KX022_CNTL1, KX022_INC1_IEN | KX022_INC1_IEA | KX022_INC1_IEL);
		ERROR_CODE_CHECK(err_code);
		
		err_code = kx022_write_byte(KX022_INC4, KX022_INC4_DRDYI);
		ERROR_CODE_CHECK(err_code);
		
		err_code = kx022_sensor_set_sample_rate(rate);
		ERROR_CODE_CHECK(err_code);
		
		err_code = kx022_sensor_set_accel_fsr(fsr);
	}
	
	return err_code;
}

uint8_t kx022_sensor_disable()
{
	uint8_t err_code;
	
	err_code = spi_disable();
	if(err_code == NRF_SUCCESS)
		is_spi_active = false;
	
	return err_code;
}

uint8_t kx022_sensor_set_accel_fsr(uint8_t fsr)
{
	return kx022_write_byte(KX022_CNTL1, KX022_CNTL1_PC1 | KX022_CNTL1_RES | KX022_CNTL1_DRDYE | fsr);
}

uint8_t kx022_sensor_set_sample_rate(uint16_t rate)
{
	return kx022_write_byte(KX022_ODCNTL, KX022_ODCNTL_IIR_BYPASS | KX022_ODCNTL_LPRO | rate);
}

uint8_t kx022_sensor_get_accel_fsr(uint8_t *fsr)
{
	uint32_t err_code;
	
	err_code = kx022_read_byte(KX022_CNTL1, fsr, 2);
	fsr[1] = fsr[1] & 0x18;
	
	return err_code;
}

uint8_t kx022_sensor_get_sample_rate(uint16_t *rate)
{
	uint32_t err_code;
	
	err_code = kx022_read_byte(KX022_ODCNTL, rate, 2);
	rate[1] = rate[1] & 0x07;
	
	return err_code;
}

uint8_t kx022_sensor_get_accel(uint8_t *rx_data)
{
	return kx022_read(KX022_XOUTL, rx_data, 7);
}

uint8_t kx022_who_am_i(uint8_t *rx_data)
{	
	return kx022_read(KX022_WHO_AM_I, rx_data, 2);
}


