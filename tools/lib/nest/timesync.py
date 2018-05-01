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
from util import NestOpcode
from util import NestSerialProcess
from lib.projutil import ProjectUtil

class NestTimeSyncProcess(NestProcess):
    def __init__(self, port, ipaddr, path):
        NestProcess.__init__(self)
        timestamp = int(time.time())
        attr = struct.pack(">i", timestamp)
        arr = bytearray(4)
        for i in range(0, 4):
            arr[i] = attr[i]

        if port is not '':
            self.nsp = NestSerialProcess(NestSerialProcess.NEST_INTERFACE_COM, port, self.data_received)
        elif ipaddr is not '':
            self.nsp = NestSerialProcess(NestSerialProcess.NEST_INTERFACE_TCP, ipaddr, self.data_received)

        command = self.nsp.command_pack(arr, NestOpcode.CORE_TIME_SYNC)
        self.nsp.send(command)

    def data_received(self, data):
        print 'NestTimeSyncProcess data received'
        command = self.nsp.command_unpack(data)
        if not command == None:
            if command.opcode == (NestOpcode.CORE_TIME_SYNC | 0x80):
                if command.data_len == 1:
                    if command.data[0] == 0x00:
                        print 'time is successfully synchronized'
        self.dispose()

    def abort_handler(self):
        self.nsp.close()
        exit()
