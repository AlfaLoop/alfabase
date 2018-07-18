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
#include "hw_api.h"
#include "frameworks/app_lifecycle.h"
#include "frameworks/app_eventpool.h"
#include "frameworks/hw/hw_api_null.h"
#include "loader/symtab.h"
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
static HWDriver m_null_hwdriver;
/*---------------------------------------------------------------------------*/
HWDriver*
HWPipe(const char *dev)
{
	if (hw_api_bsp_num() == 0)
		return &m_null_hwdriver;
	if (hw_api_bsp_pipe(dev) == NULL)
		return &m_null_hwdriver;

	return hw_api_bsp_pipe(dev);
}
static struct symbols symbolHWPipe = {
	.name = "HWPipe",
	.value = (void *)&HWPipe
};
/*---------------------------------------------------------------------------*/
HWDriver*
HWGet(uint32_t idx)
{
	if (hw_api_bsp_num() == 0 || idx >= hw_api_bsp_num())
		return &m_null_hwdriver;
	return hw_api_bsp_get(idx);
}
static struct symbols symbolHWGet = {
	.name = "HWGet",
	.value = (void *)&HWGet
};
/*---------------------------------------------------------------------------*/
int HWNum(void)
{
	return hw_api_bsp_num();
}
static struct symbols symbolHWNum = {
	.name = "HWNum",
	.value = (void *)&HWNum
};
/*---------------------------------------------------------------------------*/
void
hw_api_init(void)
{
	// initialate null instance
	m_null_hwdriver.name = "null";
	m_null_hwdriver.open = hw_null_open;
	m_null_hwdriver.write = hw_null_write;
	m_null_hwdriver.read = hw_null_read;
	m_null_hwdriver.subscribe = hw_null_subscribe;
	m_null_hwdriver.close = hw_null_close;

	symtab_add(&symbolHWGet);
	symtab_add(&symbolHWPipe);
	symtab_add(&symbolHWNum);
}
/*---------------------------------------------------------------------------*/
