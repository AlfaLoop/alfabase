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
#ifndef _ADC_H
#define _ADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define ADC_MODE_BLOCK     1
#define ADC_MODE_NONBLOCK  2

typedef void (* adc_cb_t)(uint32_t pin, uint32_t value);
typedef int (* adc_channel_cb_t)(void * config);

typedef struct {
  uint8_t mode;
  adc_cb_t cb;
} adc_config_t;

/**
 * The structure of a ADC driver
 */
struct adc_driver {
	char *name;
	int (* init)(adc_config_t *config);
  int (* channel_init)(uint32_t pin, void *config);
  int16_t (* sample)(uint32_t pin);
	int (* channel_uninit)(uint32_t pin);
};

#ifdef __cplusplus
}
#endif
#endif /* _ADC_H */
