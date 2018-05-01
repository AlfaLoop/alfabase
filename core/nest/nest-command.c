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
#include <string.h>
#include <ctype.h>
#include "nest.h"
#include "libs/util/linklist.h"
#include "frameworks/app_lifecycle.h"
#include "frameworks/app_eventpool.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

/*---------------------------------------------------------------------------*/
#if defined(DEBUG_ENABLE)
#define DEBUG_MODULE 1
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
#ifndef NEST_CONF_COMMAND_RXQ_ITEMS
# define RX_QUEUE_SIZE 10
#else
# define RX_QUEUE_SIZE NEST_CONF_COMMAND_RXQ_ITEMS
#endif
/*---------------------------------------------------------------------------*/
LIST(nest_commands);
static struct process *front_process;
PROCESS(nest_command_process, "Nest command");
/*---------------------------------------------------------------------------*/

typedef enum { FREE=0x0, INPUT, OUTPUT, INPUT_ALLOC, OUTPUT_ALLOC } rxqueue_types_t;

/* storage for queues */
static nest_command_data_t rxbuf[RX_QUEUE_SIZE] ;

typedef struct {
	uint8_t head, tail, types[RX_QUEUE_SIZE];
} rxq_t;
static rxq_t rxq;

/*---------------------------------------------------------------------------*/
static void
rxq_head_complete(void)
{
	switch(rxq.types[rxq.head])
	{
		case INPUT_ALLOC:
			rxq.types[rxq.head] = INPUT;
		break;
		case OUTPUT_ALLOC:
			rxq.types[rxq.head] = OUTPUT;
		break;
		case FREE:
		break;
		default:
		break;
	}
}
/*---------------------------------------------------------------------------*/
static void
rxq_enqueue(nest_command_data_t *ind, rxqueue_types_t type)
{
	if(rxq.types[rxq.tail] == FREE)
	{
		rxq.types[rxq.tail] = type;
		rxbuf[rxq.tail].opcode = ind->opcode;
		rxbuf[rxq.tail].len = ind->len;
		rxbuf[rxq.tail].response_process = ind->response_process;
		memcpy(rxbuf[rxq.tail].data, ind->data, ind->len);
		rxq.tail = (rxq.tail+1)%RX_QUEUE_SIZE;
		// PRINTF("[nest command] rxq enqueue tail=%d \n", rxq.tail);
	}
}
/*---------------------------------------------------------------------------*/
static void
rxq_dequeue()
{
	if(rxq.types[rxq.head] == INPUT ||
		 rxq.types[rxq.head] == OUTPUT)
	{
		rxq.types[rxq.head] = FREE;
		rxq.head = (rxq.head+1)%RX_QUEUE_SIZE;
	}
}
/*---------------------------------------------------------------------------*/
/* rx queue handling functions */
static nest_command_data_t*
rxq_peek()
{
	// PRINTF("[nest command] rxq peak head=%d type=%d\n", rxq.head, rxq.types[rxq.head]);
  if(rxq.types[rxq.head]== FREE ||
		 rxq.types[rxq.head]== INPUT_ALLOC ||
     rxq.types[rxq.head]== OUTPUT_ALLOC )
    return NULL;

  return (nest_command_data_t*) &rxbuf[rxq.head];
}
/*---------------------------------------------------------------------------*/
static rxqueue_types_t
rxq_peektype()
{
	return rxq.types[rxq.head];
}
/*---------------------------------------------------------------------------*/
static void
nest_start_command(nest_command_data_t *input, struct process **started_process)
{
	struct nest_command *c;

	if(started_process != NULL) {
		*started_process = NULL;
    }

	// PRINTF("[nest command] opcode:0x%02x\n", input->opcode);
	for(c = list_head(nest_commands);
		c != NULL &&
		c->opcode != input->opcode;
		c = c->next);

	if(c == NULL) {
		// PRINTF("0x%2x command not found \n", c->opcode);
	} else if(process_is_running(c->process) ) {
		// PRINTF("0x%2x command already running \n", c->opcode);
		c = NULL;
	} else {
		// PRINTF("[nest command] starting '%s'\n", c->process->name);
		/* Start a new process for the command. */
		process_start(c->process, input);
		*started_process = c->process;
	}
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_command_process, ev, data)
{
	static struct process *started_process;
	static struct etimer timeout_etimer;
	static nest_command_data_t *cmd_data = NULL;

	PROCESS_BEGIN();
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_POLL) );
		do {
			rxq_head_complete();
			if ( rxq_peektype() == INPUT )
			{
				cmd_data = rxq_peek();
				if (cmd_data != NULL) {
					// PRINTF("[nest command] INPUT opcode %d len %d\n", cmd_data->opcode, cmd_data->len);
					nest_start_command(cmd_data, &started_process);

					if(started_process != NULL && process_is_running(started_process)) {
						front_process = started_process;
						// PRINTF("start process's name %s\n", front_process->name);
#if NEST_COMMAND_LUNCHR_ENABLE_CONF == 1
						// setup the timeout
						if (started_process == &nest_lunchr_run_with_uuid_process) {
							etimer_set(&timeout_etimer, CLOCK_SECOND * 30);
						} else {
							etimer_set(&timeout_etimer, CLOCK_SECOND * 5);
						}
#else
						etimer_set(&timeout_etimer, CLOCK_SECOND * 5);
#endif
						PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_EXITED && data == started_process) ||
																			(etimer_expired(&timeout_etimer)) );

						if (etimer_expired(&timeout_etimer)) {
							if (front_process != &nest_command_process) {
								process_exit(front_process);
							}
						} else {
							etimer_stop(&timeout_etimer);
						}
						// PRINTF("end process's name %s\n", front_process->name);
					}
					front_process = &nest_command_process;
				}
			}
			else if ( rxq_peektype() == OUTPUT )
			{
				cmd_data = rxq_peek();
				if (cmd_data != NULL) {
					PRINTF("[nest command] OUTPUT opcode %d len %d\n", cmd_data->opcode, cmd_data->len);
					if (cmd_data->response_process != NULL ) {
						process_start(cmd_data->response_process, (void *)cmd_data);
						if (process_is_running(cmd_data->response_process)) {
							front_process = cmd_data->response_process;
							etimer_set(&timeout_etimer, CLOCK_SECOND * 5);
							PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_EXITED && data == cmd_data->response_process) ||
																				(etimer_expired(&timeout_etimer)) );
							if (etimer_expired(&timeout_etimer)) {
								if (front_process != &nest_command_process) {
									process_exit(front_process);
								}
							} else {
								etimer_stop(&timeout_etimer);
							}
							front_process = &nest_command_process;
						}
					}
				}
			}
			rxq_dequeue();
		} while( rxq_peektype() != FREE );
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
nest_command_init(void)
{
	list_init(nest_commands);
	memset(rxbuf, 0x00, sizeof(nest_command_data_t) * RX_QUEUE_SIZE);
	memset(&rxq, 0x00, sizeof(rxq_t));

	process_start(&nest_command_process, NULL);
	front_process = &nest_command_process;
}
/*---------------------------------------------------------------------------*/
void
nest_command_unregister(struct nest_command *c)
{
	list_remove(nest_commands, c);
}
/*---------------------------------------------------------------------------*/
void
nest_command_register(struct nest_command *c)
{
	struct nest_command *i, *p;

	p = NULL;
	for(i = list_head(nest_commands);
		i != NULL &&
		(i->opcode != c->opcode);
		i = i->next)
	{
		p = i;
	}

	if(p == NULL) {
		list_push(nest_commands, c);
	}
	else if(i == NULL) {
		list_add(nest_commands, c);
	}
	else {
		list_insert(nest_commands, p, c);
	}
}
/*---------------------------------------------------------------------------*/
void
nest_command_send(nest_command_data_t *output)
{
	// PRINTF("[nest command] send\n");
	rxq_enqueue(output, OUTPUT_ALLOC);
	process_post(&nest_command_process, PROCESS_EVENT_POLL, NULL);
}
/*---------------------------------------------------------------------------*/
void
nest_command_outbound_enqueue(nest_command_data_t *output)
{
	// PRINTF("[nest command] outbound enqueue\n");
	rxq_enqueue(output, OUTPUT_ALLOC);
}
/*---------------------------------------------------------------------------*/
void
nest_command_outbound_process(void)
{
	// PRINTF("[nest command] outband process\n");
	process_post(&nest_command_process, PROCESS_EVENT_POLL, NULL);
}
/*---------------------------------------------------------------------------*/
void
nest_command_inbound(nest_command_data_t *ind)
{
	// PRINTF("[nest command] inbound\n");
	if(process_is_running(&nest_command_process)) {
		// copy input to buffer queue, and leave the interrupt content
		rxq_enqueue(ind, INPUT_ALLOC);
		process_post(&nest_command_process, PROCESS_EVENT_POLL, NULL);
  }
}
/*---------------------------------------------------------------------------*/
