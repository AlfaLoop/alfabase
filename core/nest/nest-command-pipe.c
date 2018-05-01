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
