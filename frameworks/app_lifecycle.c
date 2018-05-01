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
#include "app_lifecycle.h"
#include "libs/util/linklist.h"
#include "errno.h"
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
LIST(lifecycle_list);
/*---------------------------------------------------------------------------*/
void
app_lifecycle_notify(uint8_t opcode)
{
	struct app_lifecycle_event *s;
	for(s = list_head(lifecycle_list);
		s != NULL;
		s = list_item_next(s))
	{
		if (opcode == APP_LIFECYCLE_TERMINATE) {
			PRINTF("opcode %d name:%s\n", opcode, s->name);
			s->terminating();
		}
	}
}
/*---------------------------------------------------------------------------*/
int
app_lifecycle_register(struct app_lifecycle_event *events)
{
	if (events == NULL)
		return EINVAL;

	list_add(lifecycle_list, events);
	return ENONE;
}
/*---------------------------------------------------------------------------*/
void
app_lifecycle_init(void)
{
	/* code */
	list_init(lifecycle_list);
}
/*---------------------------------------------------------------------------*/
