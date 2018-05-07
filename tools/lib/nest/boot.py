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

class NestSetBootProcess(NestProcess):
    def __init__(self, port, ipaddr, path):
        NestProcess.__init__(self)
        util = ProjectUtil()
        self.nsp = None
        self.conf_content = util.load_conf(path, True)

        if port is not '':
            self.nsp = NestSerialProcess(NestSerialProcess.NEST_INTERFACE_COM, port, self.data_received)
        elif ipaddr is not '':
            self.nsp = NestSerialProcess(NestSerialProcess.NEST_INTERFACE_TCP, ipaddr, self.data_received)

        # initiate run packetes
        command = self.nsp.command_pack(None, NestOpcode.LUNCHR_SET_BOOT_PROCESS)
        self.nsp.send(command)

    def abort_with_error(self, err_code):
        if err_code == NestErrCode.INVALID_STATE:
            print 'Invalid status'
        elif err_code == NestErrCode.ENULLP:
            print 'File not found'
        elif err_code == NestErrCode.EINTERNAL:
            print 'Internal error'
        self.dispose()

    def data_received(self, data):
        command = self.nsp.command_unpack(data)
        if not command == None:
            if command.opcode == (NestOpcode.LUNCHR_SET_BOOT_PROCESS | 0x80):
                if command.data_len == 5:
                    #　Get the BFTP Init response, start to issue the first raw packets
                    if command.data[0] == 0x00:
                        uuid_attr = command.data[1:5]
                        print 'set application {} as boot program'.format(uuid_attr)
                        self.dispose()
                    else:
                        self.abort_with_error(command.data[0])

    def abort_handler(self):
        if self.nsp is not None:
            self.nsp.close()
        exit()


class NestDelBootProcess(NestProcess):
    def __init__(self, port, ipaddr, path):
        NestProcess.__init__(self)
        util = ProjectUtil()
        self.nsp = None
        self.conf_content = util.load_conf(path, True)

        if port is not '':
            self.nsp = NestSerialProcess(NestSerialProcess.NEST_INTERFACE_COM, port, self.data_received)
        elif ipaddr is not '':
            self.nsp = NestSerialProcess(NestSerialProcess.NEST_INTERFACE_TCP, ipaddr, self.data_received)

        # initiate run packetes
        command = self.nsp.command_pack(None, NestOpcode.LUNCHR_REMOVE_BOOT_PROCESS)
        self.nsp.send(command)

    def data_received(self, data):
        command = self.nsp.command_unpack(data)
        if not command == None:
            if command.opcode == (NestOpcode.LUNCHR_REMOVE_BOOT_PROCESS | 0x80):
                if command.data_len == 1:
                    print 'remove boot program'
                    self.dispose()

    def abort_handler(self):
        if self.nsp is not None:
            self.nsp.close()
        exit()
