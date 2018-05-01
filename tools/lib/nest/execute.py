# -*- coding: utf-8 -*-
import serial
import sys
import os
import struct
import json
import time
import threading
import signal

from util import NestProcess
from util import NestErrCode
from util import NestOpcode
from util import NestSerialProcess
from lib.projutil import ProjectUtil

class NestExecuteProcess(NestProcess):
    def __init__(self, port, ipaddr, path):
        NestProcess.__init__(self)
        util = ProjectUtil()
        self.nsp = None
        self.conf_content = util.load_conf(path, True)
        self.short_uuid = self.conf_content['short_uuid']
        self.app_name = self.conf_content['name']

        attr = bytearray(4)
        attr[0] = int(self.short_uuid[0:2], 16)
        attr[1] = int(self.short_uuid[2:4], 16)
        attr[2] = int(self.short_uuid[4:6], 16)
        attr[3] = int(self.short_uuid[6:8], 16)

        if port is not '':
            self.nsp = NestSerialProcess(NestSerialProcess.NEST_INTERFACE_COM, port, self.data_received)
        elif ipaddr is not '':
            self.nsp = NestSerialProcess(NestSerialProcess.NEST_INTERFACE_TCP, ipaddr, self.data_received)

        # initiate run packetes
        command = self.nsp.command_pack(attr, NestOpcode.LUNCHR_EXEC)
        self.nsp.send(command)

    def abort_with_error(self, err_code):
        if err_code == NestErrCode.NULL_POINTER:
            print 'Null pointer'
        elif err_code == NestErrCode.BUSY:
            print 'Device Busy'
        elif err_code == NestErrCode.ELF_NO_SYMBOL_TABLE:
            print 'No symbol table'
        elif err_code == NestErrCode.FILE_NO_SUCH_FILE:
            print 'No such application on edge, deploy it again'
        elif err_code == NestErrCode.ELF_SYMBOL_NOT_FOUND:
            print 'Symbol not found, app will be removed on edge device'
        elif err_code == NestErrCode.ELF_BAD_ELF_HEADER:
            print 'Bad elf header, app will be removed on edge device, deploy it again'
        self.dispose()

    def data_received(self, data):
        command = self.nsp.command_unpack(data)
        if not command == None:
            if command.opcode == (NestOpcode.LUNCHR_EXEC | 0x80):
                if command.data_len == 1:
                    #　Get the BFTP Init response, start to issue the first raw packets
                    if command.data[0] == 0x00:
                        print 'application {} start'.format(self.app_name)
                        self.dispose()
                    else:
                        self.abort_with_error(command.data[0])

    def abort_handler(self):
        if self.nsp is not None:
            self.nsp.close()
        exit()
