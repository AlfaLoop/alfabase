# -*- coding: utf-8 -*-
import sys
import os
import json
import subprocess
import uuid
import base64
import shutil


'''
app.conf helloworld example
app.conf model
{
    "name" : "helloworld",
	"author" : "",
	"uuid" : "7e23acfc-1ed8-5b3a-9d2f-a872cd515730",
	"short_uuid" : "0da768fb",
	"source" : "["main.c"]",
	"include_dir" : "[".", "../"]",
	"cflags" : "[]"
}
'''


ENTEY_PROGRAM = '''#include "prefix.h"
extern int main(void);
PROCESS(up1, "up1");
AUTOSTART_PROCESSES(&up1);
PROCESS_THREAD(up1, ev, data)
{
PROCESS_BEGIN();
OSProcessEntrySetup(main);
while (1) {
PROCESS_WAIT_EVENT();
}
PROCESS_END();
}
'''

ELFLD = '''SECTIONS
{
	.text 0x00000000 :
	{
		*(.text)
		*(.rel.text)
	}
	.data :
	{
		*(.data)
		*(.rel.data)
	}
	.bss :
	{
		*(.bss)
	}
	.rodata :
	{
		*(.rodata)
		*(.rel.rodata)
	}
}
'''

HELLOWORLD ='''#include "alfabase.h"

static bool timer_flag = false;
static uint32_t counter = 0;

static void
timer_event_handler(void)
{
  timer_flag = true;
}
/*---------------------------------------------------------------------------*/
int main(void)
{
  Process *process = OSProcess();
  Logger *logger = OSLogger();

  // Get the timer instance 0
  Timer *t = OSTimer();

  // Timer 0
  t->start(0, 1000, timer_event_handler);

  while (1) {
    // Power saving
    process->waitForEvent();

    if (timer_flag) {
      timer_flag = false;
      counter++;
      logger->printf(LOG_SERIAL, "Hello Timer %d\\n", counter);
    }
  }
  return 0;
}
'''

class ProjectUtil:
    def load_conf(self, path, abort=False):
        conf_path = os.path.join(path, 'app.conf')
        ctx = json.load(open(conf_path))
        t = 'name' in ctx and 'author' in ctx and 'uuid' in ctx and 'short_uuid' in ctx and \
            'source' in ctx and 'include_dir' in ctx and 'cflags' in ctx
        if not t:
            print 'app.conf file incompatible.\n'
            if abort:
                quit()
            return None
        return ctx

class ProjectCmd:
    def __init__(self, cmd, path):
        self.curr_path = path

        if cmd == 'new':
            self._gen_new_project()
        elif cmd == 'build':
            util = ProjectUtil()
            self.conf_content = util.load_conf(path, True)
            self.cpu_type = 'nrf52832'  # default value
            self._gen_entry_program()
            self._gen_linker_file()
            self._start_compile()
            shutil.rmtree(os.path.join(self.curr_path, 'build'))
        if cmd == 'clean':
            if os.path.exists(os.path.join(self.curr_path, 'build')):
                shutil.rmtree(os.path.join(self.curr_path, 'build'))
            files = os.listdir(self.curr_path)
            for item in files:
                if item.endswith(".elf"):
                    os.remove(os.path.join(self.curr_path, item))

    def _verify(self):
        ctx = self.conf_content
        t = 'name' in ctx and 'author' in ctx and 'uuid' in ctx and 'short_uuid' in ctx and \
            'source' in ctx and 'include_dir' in ctx and 'cflags' in ctx
        if not t:
            print 'app.conf file incompatible.\n'
            quit()

    def _gen_new_project(self):
        project_name = raw_input("Enter project name: ")
        if project_name == '':
            project_name = 'hello'

        author_name = raw_input("Enter author name: ")
        main_program = os.path.join(self.curr_path, project_name, 'main.c')
        conf_file = os.path.join(self.curr_path, project_name, 'app.conf')

        devuuid = '0017a55a-8190-eff1-9b2e-20cf2ab43099'
        seed = '{}:{}:{}'.format(devuuid, project_name, author_name)
        long_uuid = uuid.uuid5(uuid.NAMESPACE_DNS, seed)

        long_uuid_hex = long_uuid.hex
        short_uuid_str = format(int(long_uuid_hex[0:2], 16) ^ int(long_uuid_hex[2:4], 16) ^ \
            int(long_uuid_hex[4:6], 16) ^ int(long_uuid_hex[6:8], 16), '02x')
        short_uuid_str += format(int(long_uuid_hex[8:10], 16) ^ int(long_uuid_hex[10:12], 16) ^ \
            int(long_uuid_hex[12:14], 16) ^ int(long_uuid_hex[14:16], 16), '02x')
        short_uuid_str += format(int(long_uuid_hex[16:18], 16) ^ int(long_uuid_hex[18:20], 16) ^ \
            int(long_uuid_hex[20:22], 16) ^ int(long_uuid_hex[22:24], 16), '02x')
        short_uuid_str += format(int(long_uuid_hex[24:26], 16) ^ int(long_uuid_hex[26:28], 16) ^ \
            int(long_uuid_hex[28:30], 16) ^ int(long_uuid_hex[30:32], 16), '02x')

        # generate app.conf file
        if not os.path.exists(os.path.dirname(conf_file)):
            try:
                os.makedirs(os.path.dirname(conf_file))
            except OSError as exc: # Guard against race condition
                if exc.errno != errno.EEXIST:
                    raise
        with open(conf_file, "w") as f:
            f.write('{\n')
            f.write('\t"name" : "{}",\n'.format(project_name))
            f.write('\t"author" : "{}",\n'.format(author_name))
            f.write('\t"uuid" : "{}",\n'.format(str(long_uuid)))
            f.write('\t"short_uuid" : "{}",\n'.format(short_uuid_str))
            f.write('\t"source" : {},\n'.format('["main.c"]'))
            f.write('\t"include_dir" : {},\n'.format('[".", "../"]'))
            f.write('\t"cflags" : {}\n'.format('[]'))
            f.write('}\n')
            f.close()

        # generate main.c file
        if not os.path.exists(os.path.dirname(main_program)):
            try:
                os.makedirs(os.path.dirname(main_program))
            except OSError as exc: # Guard against race condition
                if exc.errno != errno.EEXIST:
                    raise
        with open(main_program, "w") as f:
            f.write(HELLOWORLD)
            f.close()

    def _gen_entry_program(self):
        # gen prefix.h
        prefixfile = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'prefix.h')
        if not os.path.exists(os.path.join(self.curr_path, 'build')):
            try:
                os.makedirs(os.path.join(self.curr_path, 'build'))
            except OSError as exc: # Guard against race condition
                if exc.errno != errno.EEXIST:
                    raise
        shutil.copy2(prefixfile, os.path.join(self.curr_path, 'build', 'prefix.h'))

        # gen start.c unter build folder
        start_program = os.path.join(self.curr_path, 'build', 'start.c')
        if not os.path.exists(os.path.dirname(start_program)):
            try:
                os.makedirs(os.path.dirname(start_program))
            except OSError as exc: # Guard against race condition
                if exc.errno != errno.EEXIST:
                    raise
        with open(start_program, "w") as f:
            f.write(ENTEY_PROGRAM)
            f.close()

    def _gen_linker_file(self):
        # gen start.c unter build folder
        ldfile = os.path.join(self.curr_path, 'build', 'elf.ld')
        if not os.path.exists(os.path.dirname(ldfile)):
            try:
                os.makedirs(os.path.dirname(ldfile))
            except OSError as exc: # Guard against race condition
                if exc.errno != errno.EEXIST:
                    raise
        with open(ldfile, "w") as f:
            f.write(ELFLD)
            f.close()

    def _start_compile(self):
        ctx = self.conf_content
        objs = []
        # compile the start.o
        ldfile = os.path.join(self.curr_path, 'build', 'elf.ld')
        tmpelf = os.path.join(self.curr_path, 'build', 'tmp.elf')
        startc = os.path.join(self.curr_path, 'build', 'start.c')
        starto = os.path.join(self.curr_path, 'build', 'start.o')
        elf = os.path.join(self.curr_path,  ctx['name'] + '-app.elf')
        self._compile_source(starto, startc)

        objs.append(starto)

        # compile the project sources
        for s in ctx['source']:
            filename, file_extension = os.path.splitext(s)
            print 'compile ' + s
            self._compile_source(os.path.join(self.curr_path, 'build', filename + '.o'), s)
            objs.append(os.path.join(self.curr_path, 'build', filename + '.o'))

        print 'link....'
        # link
        self._ldelf(ldfile, objs, tmpelf)

        # strip
        print 'strip.....'
        self._stripelf(tmpelf, elf)
        print 'Total size'
        self._sizeelf(ctx['name'] + '-app.elf')

    def _sizeelf(self, src):
        attr = ['arm-none-eabi-size', '--common', src]
        cmd = " ".join(attr)
        subprocess.call(cmd, stderr=subprocess.STDOUT, shell=True)


    def _stripelf(self, src, dst):
        attr = ['arm-none-eabi-strip', '-g', '-o', dst, '-x', src]
    	cmd = " ".join(attr)
        p = subprocess.Popen(cmd)
        p.wait()

    def _ldelf(self, ldfile, objs, dst):
        ctx = self.conf_content

        attr = ['arm-none-eabi-gcc', '-r', '-Bsymbolic', '-nostartfiles', '-fno-common', '-mthumb',
        '-mabi=aapcs', '-mlittle-endian', '-fno-merge-constants', '-fno-function-sections',
        '-DAUTOSTART_ENABLE', '-T', ldfile, '-o', dst]

        if self.cpu_type == 'nrf52832':
            attr.append('-mfloat-abi=hard')
            attr.append('-mfpu=fpv4-sp-d16')
            attr.append('-mcpu=cortex-m4')
        elif self.cpu_type == 'nrf51822':
            attr.append('-mfloat-abi=soft')
            attr.append('-mcpu=cortex-m0')

        # add -I flag
        for i in ctx['include_dir']:
            attr.append('-I' + i)

        # add -D flags
        for i in ctx['cflags']:
            attr.append(i)

        # add object files
        for o in objs:
            attr.append(o)

        cmd = " ".join(attr)
        p = subprocess.Popen(cmd)
        p.wait()

    def _compile_source(self, dst, src):
        """
        arm-none-eabi-gcc -MMD -Og -ggdb --std=gnu99 -Wall -mthumb -mabi=aapcs -mlittle-endian -fno-merge-constants -DAUTOSTART_ENABLE -w -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mcpu=cortex-m4 -I. -I../ -o D:\Project\Github\AlfaLoop\iotaos\apps\helloworld\build\start.o -c D:\Project\Github\AlfaLoop\iotaos\apps\helloworld\build\start.c
        """
        substring = 'error'
        ctx = self.conf_content
        gcc = 'arm-none-eabi-gcc '
        attr = ['-MMD', '-Og', '-ggdb', '--std=gnu99', '-Wall', '-mthumb',
        '-mabi=aapcs', '-mlittle-endian', '-fno-merge-constants',
        '-DAUTOSTART_ENABLE', '-w']

        if self.cpu_type == 'nrf52832':
            attr.append('-mfloat-abi=hard')
            attr.append('-mfpu=fpv4-sp-d16')
            attr.append('-mcpu=cortex-m4')
        elif self.cpu_type == 'nrf51822':
            attr.append('-mfloat-abi=soft')
            attr.append('-mcpu=cortex-m0')

        # add -I flag
        for i in ctx['include_dir']:
            attr.append('-I' + i)

        # add -D flags
        for i in ctx['cflags']:
            attr.append(i)

        attr.append('-o')
        attr.append(dst)

        attr.append('-c')
        attr.append(src)

        cmd = " ".join(attr)
        process = subprocess.Popen(gcc + cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)
        stdout, stderr = process.communicate(input='aweimeow\n')
        if stderr.find(substring) is not -1:
            print stderr
            exit()
