/**
 * © Copyright AlfaLoop Technology Co., Ltd. 2018
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/
#ifndef _GPIOTE_H_
#define _GPIOTE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define NO_OF_PINS              32          /**< Number of GPIO pins on the nRF52 chip. */

typedef struct {
	uint32_t                   pin_no;
	uint32_t                   pins_low_to_high_mask;
	uint32_t                   pins_high_to_low_mask;
} gpiote_event_t;

typedef struct gpiote_handle {
	struct gpiote_handle 	  *next;
	uint32_t                   pins_mask;             				  /**< Mask defining which pins user wants to monitor. */
	uint32_t                   sense_high_pins;       				  /**< Mask defining which pins are configured to generate GPIOTE interrupt on transition to high level. */
	uint32_t                   pins_low_to_high_mask;			      /**< Mask defining which pins will generate events to this user when toggling low->high. */
	uint32_t                   pins_high_to_low_mask;			      /**< Mask defining which pins will generate events to this user when toggling high->low. */
	void              		  	 (*event_handler)(gpiote_event_t *event);	  				  /**< Callback function. */
} gpiote_handle_t;


void gpiote_init(void);
void gpiote_register(struct gpiote_handle *);
void gpiote_unregister(struct gpiote_handle *);

#ifdef __cplusplus
}
#endif
#endif /* _GPIOTE_H_  */
