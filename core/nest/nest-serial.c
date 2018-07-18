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
#include "nest.h"
#include <string.h>
#include "errno.h"
#include "nest.h"
#include "dev/watchdog.h"
#include "libs/util/ringbuf.h"
#include "sys/etimer.h"
#include "bsp_init.h"
/*---------------------------------------------------------------------------*/
#if defined(DEBUG_ENABLE)
#define DEBUG_MODULE 0
#if DEBUG_MODULE
#include "dev/syslog.h"
#define PRINTF(...) syslog(__VA_ARGS__)
#else
#define PRINTF(...)
#endif  /* DEBUG_MODULE */
#else
#define PRINTF(...)
#endif  /* DEBUG_ENABLE */
/*---------------------------------------------------------------------------*/
#ifdef NEST_SERIAL_LINE_CONF_BUFSIZE
#define BUFSIZE NEST_SERIAL_LINE_CONF_BUFSIZE
#else /* EBB_LINE_CONF_BUFSIZE */
#define BUFSIZE 128
#endif /* EBB_LINE_CONF_BUFSIZE */

#if (BUFSIZE & (BUFSIZE - 1)) != 0
#error NEST_SERIAL_LINE_CONF_BUFSIZE must be a power of two (i.e., 1, 2, 4, 8, 16, 32, 64, ...).
#error Change NEST_SERIAL_LINE_CONF_BUFSIZE in bsp_init.h.
#endif

#define NEST_SERIA_DATA_LEN      20
#define NEST_COMMAND_DATA_LEN    16

#define PREFIX_A 		0xA5
#define PREFIX_B 		0x5A
#define END			    0xF5

static struct ringbuf rxbuf;
static uint8_t rxbuf_data[BUFSIZE];
static uint8_t rxstatus = 0;
static int rxptr = 0;
static int rxlen = 0;
/*---------------------------------------------------------------------------*/
static void
nest_serial_ctimer_callback_handler(void)
{
  rxstatus = 0;
  rxlen = 0;
  rxptr = 0;
}
/*---------------------------------------------------------------------------*/
PROCESS(nest_serial_send_process, "Nest serial send");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_serial_send_process, ev, data)
{
  static nest_command_data_t *output;
  static uint8_t txbuf[NEST_SERIA_DATA_LEN + 4];

  PROCESS_BEGIN();

  output = data;

  txbuf[0] = PREFIX_A;
  txbuf[1] = PREFIX_B;
  txbuf[2] = 21;   // TODO
  nest_packer_pack(&txbuf[3], output->len, output->data, output->opcode | 0x80);
  txbuf[NEST_SERIA_DATA_LEN + 3] = END;
  nest_serial_bsp_send(&txbuf[0], NEST_SERIA_DATA_LEN + 4);
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS(nest_serial_server_process, "Nest serial server");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_serial_server_process, ev, data)
{
  static char buf[BUFSIZE];
  static struct ctimer ct;
  static nest_command_data_t input;
  static nest_proto_header_t header;

  PROCESS_BEGIN();

  rxptr = 0;
  while(1) {
    PROCESS_YIELD();
    // get the total elements
    int j = ringbuf_elements(&rxbuf);
    // PRINTF("[nest serial] process element num %d\n", j);
    for (int i = 0; i < j; i++) {
      int c = ringbuf_get(&rxbuf);
      if (rxstatus == 0) {
        if(c == PREFIX_A) {
          rxstatus = 1;
          ctimer_set(&ct, CLOCK_SECOND, nest_serial_ctimer_callback_handler, (void *)NULL);
        }
      } else if (rxstatus == 1) {
        if(c == PREFIX_B) {
          rxstatus = 2;
        } else
          ctimer_stop(&ct);
      } else if (rxstatus == 2) {
        rxptr = 0;
        if(c != -1) {
          rxlen = c;
          if (rxlen < BUFSIZE) {
            rxstatus = 3;
          } else {
            rxstatus = 0;
            ctimer_stop(&ct);
          }
        }
      } else if (rxstatus == 3) {
        if (rxptr < BUFSIZE-1 && rxptr < rxlen) {
          buf[rxptr] = (uint8_t)c;
          if (buf[rxptr] == END) {
            // PRINTF("[nest serial] unpack data len %d ", rxlen - 1);
            for (uint8_t i = 0; i < rxlen-1; i++) {
              PRINTF("%2x ", buf[i]);
            }
            PRINTF("\n");
            // parser content
            if (nest_packer_unpack(&buf[0], &input.data[0], &header)) {
              input.opcode = header.opcode;
              input.len = header.len;
              input.response_process = &nest_serial_send_process;

              // send to the command module
              nest_command_inbound(&input);
            }

            rxstatus = 0;
            ctimer_stop(&ct);
          } else {
            rxptr++;
          }
        }
      }
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
nest_serial_input(uint8_t c)
{
  if(ringbuf_put(&rxbuf, c) == 0) {
		// PRINTF("[nest serial] overflow\n");
		// overflow
		ringbuf_init(&rxbuf, rxbuf_data, sizeof(rxbuf_data));
		return;
	}
	/* Wake up consumer process */
	process_poll(&nest_serial_server_process);
}
/*---------------------------------------------------------------------------*/
void
nest_serial_init(void)
{
  ringbuf_init(&rxbuf, rxbuf_data, sizeof(rxbuf_data));

  nest_serial_bsp_enable();

  if (!process_is_running(&nest_serial_server_process))
    process_start(&nest_serial_server_process, NULL);
}
/*---------------------------------------------------------------------------*/
