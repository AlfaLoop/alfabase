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
#ifndef _NEST_COMMAND_H_
#define _NEST_COMMAND_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "contiki.h"
#include <stdint.h>
#include "nest.h"

typedef struct nest_command {
	struct nest_command *next;
	uint8_t opcode;
	struct process *process;
} nest_command_t;

/**
 * \brief      Define a nest command
 * \param name The variable name of the nest command definition
 * \param command A string with the name of the nest command
 * \param description A string that contains a one-line description of the command
 * \param process A pointer to the process that implements the nest command
 *
 *             This macro defines and declares a nest command (struct
 *             nest_command). This is used with the
 *             nest_register_command() function to register the
 *             command with the nest.
 *
  * \hideinitializer
 */
#define NEST_COMMAND(name, opcode, process) \
static nest_command_t name = { NULL, opcode, \
                                     process }

typedef void (*nest_command_response_callback_t)(uint8_t opcode, uint8_t *data, uint16_t len);

/**
 * \brief      Structure for nest input data
 *
 *             The nest sends data as Contiki events to nest command
 *             processes. This structure contains the data in the
 *             event. The data is split into two halves, data1 and
 *             data2. The length of the data in the two halves is
 *             given by len1 and len2.
 *
 */
typedef struct {
	uint8_t  opcode;
	uint8_t  data[16];
	uint16_t len;
	struct process *response_process;
} nest_command_data_t;


/**
 * \brief      Initialize the nest.
 *
 *             This function initializes the nest. It typically is
 *             called from the nest back-end.
 *
 */
void nest_command_init(void);

/**
 * \brief      Register a command with the nest
 * \param c    A pointer to a nest command structure, defined with NEST_COMMAND()
 *
 *             This function registers a nest command with the
 *             nest. After becoming registered, the nest command
 *             will appear in the list of available nest commands and
 *             is possible to invoke by a user. The nest command must
 *             have been defined with the NEST_COMMAND() macro.
 *
 */
void nest_command_register(struct nest_command *c);

/**
 * \brief      Unregister a previously registered nest command
 * \param c    A pointer to a nest command structure
 *
 *             This function unregisters a nest command that has
 *             previously been registered with eht
 *             nest_register_command() function.
 *
 */
void nest_command_unregister(struct nest_command *c);


PROCESS_NAME(nest_command_process);

void nest_command_inbound(nest_command_data_t *ind);
void nest_command_send(nest_command_data_t *output);
void nest_command_outbound_enqueue(nest_command_data_t *output);
void nest_command_outbound_process(void);

#ifdef __cplusplus
}
#endif
#endif /* _NEST_COMMAND_H_ */
