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
#include "frameworks/core/osfile_api.h"
#include "loader/symtab.h"
#include "nest/nest.h"
#include "frameworks/app_lifecycle.h"
#include "frameworks/app_eventpool.h"
#include "sys/clock.h"
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
#define OSFILE_CONCAT_BUFFER 19			// s/12345678/12345678

typedef enum {
  OM_DEFAULT = 0,
  OM_CREATE = 1,
  OM_APPEND = 2,
  OM_TRUNCATE = 4
} OpenMode;

typedef enum {
  AM_READ = 1,
  AM_WRITE = 2,
  AM_RW = AM_READ | AM_WRITE
} AccessMode;

/*---------------------------------------------------------------------------*/
static char m_concat_buffer[OSFILE_CONCAT_BUFFER];
static uint8_t m_file_open_num;
static uint16_t m_spiffs_fd[OSFILE_DATA_MAX_OPEN];
/*---------------------------------------------------------------------------*/
static int
get_spiffs_mode(OpenMode openMode, AccessMode accessMode)
{
  int mode = 0;
  if (openMode & OM_CREATE) {
    mode |= SPIFFS_CREAT;
  }
  if (openMode & OM_APPEND) {
    mode |= SPIFFS_APPEND;
  }
  if (openMode & OM_TRUNCATE) {
    mode |= SPIFFS_TRUNC;
  }
  if (accessMode & AM_READ) {
    mode |= SPIFFS_RDONLY;
  }
  if (accessMode & AM_WRITE) {
    mode |= SPIFFS_WRONLY;
  }
  return mode;
}
/*---------------------------------------------------------------------------*/
static int
file_close(const int16_t fd)
{
  if (SPIFFS_close(&SFS, fd) < 0) {
    PRINTF("[osfile] close errno %i\n", SPIFFS_errno(&SFS));
    return EINTERNAL;
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
file_seek(const int16_t fd, const uint32_t offset, const SeekMode mode)
{
  if(SPIFFS_lseek(&SFS, fd, offset, mode) < 0) {
    PRINTF("[osfile] seek errno %i\n", SPIFFS_errno(&SFS));
    return EINTERNAL;
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
file_read_data(const int16_t fd, void *data, const uint32_t size)
{
  int res;

  // read it
  if (SPIFFS_read(&SFS, fd, (uint8_t *)data, size) < 0) {
    PRINTF("[osfile] read data errno %i\n", SPIFFS_errno(&SFS));
    return EINTERNAL;
  }

	return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
file_write_data(const int16_t fd, const void *data, const uint32_t size)
{
  if (size > OSFILE_DATA_MAX_LENGTH)
    return EINVAL;

	// write to it
	if (SPIFFS_write(&SFS, fd, (uint8_t *)data, size) < 0) {
		PRINTF("[osfile] write data errno %i\n", SPIFFS_errno(&SFS));
		return EINTERNAL;
	}
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
file_open(int16_t *fd, const uint32_t key, const char *mode)
{
  uint32_t err_code = ENONE;
  uint8_t access_mode = 0;
  uint8_t open_mode = 0;
  int res;
  spiffs_file spiffs_fd;
  uint32_t uuid;

  if (lunchr_get_running_task_uuid(&uuid) != ENONE) {
    return EINTERNAL;
  }

  sprintf(m_concat_buffer, "s/%08x/%08x", uuid, key);

  switch (mode[0]) {
    case 'r':
    access_mode = AM_READ;
    open_mode = OM_DEFAULT;
    break;
    case 'w':
    access_mode = AM_WRITE;
    open_mode = (OM_CREATE | OM_TRUNCATE);
    break;
    case 'a':
    access_mode = AM_WRITE;
    open_mode = (OM_CREATE | OM_APPEND);
    break;
    default:
      err_code =  EINVAL;
  }

  switch (mode[1]) {
    case '+':
    access_mode = (AM_WRITE | AM_READ);
    break;
    case 0:
    break;
    default:
      err_code = EINVAL;
  }

  if (err_code != ENONE) {
    return err_code;
  }

  int fs_mode = get_spiffs_mode(open_mode, access_mode);
  spiffs_fd = SPIFFS_open(&SFS, m_concat_buffer, fs_mode, 0);
  if (spiffs_fd < 0) {
    PRINTF("[osfile] open errno %i\n", SPIFFS_errno(&SFS));
    return EINTERNAL;
  };
  *fd = spiffs_fd;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
file_exist(const uint32_t key)
{
  int res;
	spiffs_stat s;
  uint32_t uuid;

  if (lunchr_get_running_task_uuid(&uuid) != ENONE) {
    return EINTERNAL;
  }

  sprintf(m_concat_buffer, "s/%08x/%08x", uuid, key);

	res = SPIFFS_stat(&SFS, m_concat_buffer, &s);
	PRINTF("[osfile] exist %s res %d\n", m_concat_buffer, res);
  if (res != SPIFFS_OK) {
		return 0;
	}
	return 1;
}
/*---------------------------------------------------------------------------*/
static int
file_remove(const uint32_t key)
{
  int res;
  uint32_t uuid;

  if (lunchr_get_running_task_uuid(&uuid) != ENONE) {
    return EINTERNAL;
  }

  sprintf(m_concat_buffer, "s/%08x/%08x", uuid, key);
	res = SPIFFS_remove(&SFS, m_concat_buffer);
	if (res != SPIFFS_OK) {
		return EINTERNAL;
	}
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
file_remove_all(void)
{
  int res;
	spiffs_DIR d;
  struct spiffs_dirent e;
  struct spiffs_dirent *pe = &e;
	spiffs_file fd = -1;
  uint32_t uuid;

  if (lunchr_get_running_task_uuid(&uuid) != ENONE) {
    return EINTERNAL;
  }

  sprintf(m_concat_buffer, "s/%08x/", uuid);

	char *search_prefix = m_concat_buffer;
  SPIFFS_opendir(&SFS, "/", &d);
  while ((pe = SPIFFS_readdir(&d, pe))) {
    if (0 == strncmp(search_prefix, (char *)pe->name, 11)) {
      // found one
      fd = SPIFFS_open_by_dirent(&SFS, pe, SPIFFS_RDWR, 0);
      if (fd < 0) {
        PRINTF("open dirent errno %i\n", SPIFFS_errno(&SFS));
        SPIFFS_closedir(&d);
        return EINTERNAL;
      }
      res = SPIFFS_fremove(&SFS, fd);
      if (res < 0) {
        PRINTF("fremove errno %i\n", SPIFFS_errno(&SFS));
        SPIFFS_closedir(&d);
        return EINTERNAL;
      }
      res = SPIFFS_close(&SFS, fd);
      if (res < 0) {
        PRINTF("close errno %i\n", SPIFFS_errno(&SFS));
        SPIFFS_closedir(&d);
        return EINTERNAL;
      }
    }
  }
  SPIFFS_closedir(&d);
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
file_size(const uint32_t key)
{
	int res;
	spiffs_stat s;

  uint32_t uuid;

  if (lunchr_get_running_task_uuid(&uuid) != ENONE) {
    return EINTERNAL;
  }

  sprintf(m_concat_buffer, "s/%08x/%08x", uuid, key);

	res = SPIFFS_stat(&SFS, m_concat_buffer, &s);
	PRINTF("[osfile] size %s res %d\n", m_concat_buffer, res);
	if (res != SPIFFS_OK) {
	  return 0;
	}
  PRINTF("[osfile] size %d\n", s.size);
  return s.size;
}
/*---------------------------------------------------------------------------*/
static int
file_space(uint32_t *total, uint32_t *used)
{
  uint32_t total_space, used_space;

  if (total == NULL || used == NULL)
    return ENULLP;

  SPIFFS_info(&SFS, &total_space, &used_space);
  *total = total_space;
  *used = used_space;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
FileIO*
OSFileIO(void)
{
  static FileIO file;
  file.open = file_open;
  file.write = file_write_data;
  file.read = file_read_data;
  file.seek = file_seek;
  file.close = file_close;
  file.size = file_size;
  file.remove = file_remove;
  file.removeAll = file_remove_all;
  file.space = file_space;
  return &file;
}
static struct symbols symbolOSFileIO = {
	.name = "OSFileIO",
	.value = (void *)&OSFileIO
};
/*---------------------------------------------------------------------------*/
static void
app_terminating(void)
{
  // TODO: close unclose file
}
/*---------------------------------------------------------------------------*/
static struct app_lifecycle_event lifecycle_event = {
	.name = "osfile_api",
	.terminating = app_terminating
};
/*---------------------------------------------------------------------------*/
void
osfile_api_init(void)
{
  app_lifecycle_register(&lifecycle_event);

  m_file_open_num = 0;
  memset(m_spiffs_fd, 0x00, sizeof(int16_t) * OSFILE_DATA_MAX_OPEN);
  symtab_add(&symbolOSFileIO);
}
/*---------------------------------------------------------------------------*/
