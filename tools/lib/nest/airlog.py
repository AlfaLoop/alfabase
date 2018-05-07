# -*- coding: utf-8 -*-
import serial
import sys
import os
import struct
import json
import time
import threading
import signal
import binascii

from util import NestProcess
from util import NestErrCode
from util import NestOpcode
from util import NestSerialProcess

class NestAirLogProcess(NestProcess):
    def __init__(self, port, ipaddr):
        NestProcess.__init__(self)
        self.nsp = None
        if port is not '':
            self.nsp = NestSerialProcess(NestSerialProcess.NEST_INTERFACE_COM, port, self.data_received)
        elif ipaddr is not '':
            self.nsp = NestSerialProcess(NestSerialProcess.NEST_INTERFACE_TCP, ipaddr, self.data_received)

    def data_received(self, data):
        command = self.nsp.command_unpack(data)
        if not command == None:
            if command.opcode == (NestOpcode.PIPE_AIRLOG | 0x80):
                print str(command.data).strip()

    def abort_handler(self):
        if self.nsp is not None:
            self.nsp.close()
        exit()
