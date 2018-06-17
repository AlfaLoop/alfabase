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
#include "nest-command-auth.h"
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
PROCESS(nest_auth_echo_process, "nest-auth-echo");
NEST_COMMAND(auth_echo_command,
	      nest_air_opcode_auth_echo,
	      &nest_auth_echo_process);
PROCESS(nest_auth_reverse_process, "nest-auth-reverse");
NEST_COMMAND(auth_reverse_command,
	      nest_air_opcode_auth_reverse,
	      &nest_auth_reverse_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_auth_echo_process, ev, data)
{
	nest_command_data_t *input;
	PROCESS_BEGIN();

	input = data;

#if DEBUG_MODULE > 1
	PRINTF("nest command auth echo_process\n");
	for (uint8_t i = 0; i < input->len; i++) {
		PRINTF("%2x ", input->data[i]);
	}
	PRINTF("\n");
#endif

	nest_command_send(input);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_auth_reverse_process, ev, data)
{
	nest_command_data_t *input;
	nest_command_data_t output;
	static uint8_t packet[4];
	PROCESS_BEGIN();

	input = data;

	output.opcode = input->opcode;
	output.response_process = input->response_process;

	PRINTF("nest command auth reverse process\n");
	for (uint8_t i = 0; i < input->len; i++) {
		PRINTF("%2x ", input->data[i]);
	}
	PRINTF("\n");

	packet[0] = input->data[3];
	packet[1] = input->data[2];
	packet[2] = input->data[1];
	packet[3] = input->data[0];

	//output.data = packet;
	memcpy(output.data, packet, 4);
	output.len = 4;

	nest_command_send(&output);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
nest_command_auth_init(void)
{
	nest_command_register(&auth_echo_command);
	nest_command_register(&auth_reverse_command);
}
/*---------------------------------------------------------------------------*/
