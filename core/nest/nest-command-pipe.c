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
#include "nest-command-pipe.h"

#ifdef USE_FRAMEWORK
#endif
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

PROCESS(nest_pipe_process, "nest-pipe");
NEST_COMMAND(pipe_command,
	      nest_air_opcode_pipe,
	      &nest_pipe_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_pipe_process, ev, data)
{
	nest_command_data_t *input;
	PROCESS_BEGIN();

	input = data;
	PRINTF("nest command pipe rx process\n");
#if DEBUG_MODULE > 1
	for (uint8_t i = 0; i < input->len; i++) {
		PRINTF("%2x ", input->data[i]);
	}
	PRINTF("\n");
#endif

#if defined(USE_ELFLOADER)
	// don't need response, just bridge to upper layer
	// pipe_message_inbound(input->data, input->len, input->conn_id);
#endif

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
nest_command_pipe_init(void)
{
	nest_command_register(&pipe_command);
}
/*---------------------------------------------------------------------------*/
