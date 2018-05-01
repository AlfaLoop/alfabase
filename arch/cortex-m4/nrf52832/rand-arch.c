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
#include "contiki.h"
#include "sys/random.h"
#include "nrf.h"
/*---------------------------------------------------------------------------*/
unsigned short
random_arch_rand(void)
{
#if defined(SOFTDEVICE_PRESENT)
	uint8_t randomBytes[6] = {0};
	uint8_t bytes_available = 0;
	do {
		sd_rand_application_bytes_available_get(&bytes_available);
		bytes_available = (bytes_available > sizeof(randomBytes) ? sizeof(randomBytes) : bytes_available);
		if (bytes_available)
		{
			sd_rand_application_vector_get(randomBytes, 1);
		}

	} while(bytes_available <= 0);
	return randomBytes[0];
#else
	/* nRF5188 Product Specification v2.0, section 8.16, page 50
	 * Time to generate a byte is typically 677 us */
	NRF_RNG->EVENTS_VALRDY = 0;
	NRF_RNG->TASKS_START = 1;

	while(!NRF_RNG->EVENTS_VALRDY);

	return (uint8_t)(NRF_RNG->VALUE);
#endif
}
/*---------------------------------------------------------------------------*/
void
random_arch_init(unsigned short seed)
{
#if defined(SOFTDEVICE_PRESENT)
#else
	/* nRF51 Series Reference Manual v2.1, section 20.2, page 118
	 * Enable digital correction algorithm */
	NRF_RNG->CONFIG = RNG_CONFIG_DERCEN_Enabled << RNG_CONFIG_DERCEN_Pos;

	/* Enable shortcut to stop RNG after generating one value */
	NRF_RNG->SHORTS = RNG_SHORTS_VALRDY_STOP_Enabled <<
						RNG_SHORTS_VALRDY_STOP_Pos;
#endif
}
/*---------------------------------------------------------------------------*/
