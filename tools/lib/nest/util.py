# -*- coding: utf-8 -*-
import serial
import socket
import sys
import os
import struct
import json
import time
import threading
import signal
import binascii

NEST_SERIAL_PROTOCOL_VERSION=0x03

class NestOpcode():
    NONE = 0x00
    BFTP_INIT = 0x01
    BFTP_PACKETS = 0x02
    BFTP_END = 0x03
    BFTP_REMOVE = 0x04
    BFTP_STAT = 0x05
    BFTP_SPACE_USED = 0x06
    BFTP_READDIR = 0x07
    LUNCHR_EXEC = 0x10
    LUNCHR_KILL = 0x11
    LUNCHR_SET_BOOT_PROCESS = 0x12
    LUNCHR_REMOVE_BOOT_PROCESS = 0x13
    LUNCHR_RUNNING_QUERY = 0x14
    LUNCHR_BOOT_QUERY = 0x15
    LUNCHR_HARDFAULT_KILL = 0x16
    CORE_VERSION = 0x20
    CORE_UUID = 0x21
    CORE_PLATFORM = 0x22
    CORE_BATTERY_QUERY = 0x23
    CORE_SWITCH_BOOTLOADER = 0x24
    CORE_CHANNEL_DISCONNECT = 0x25
    CORE_TIME_SYNC = 0x26
    PIPE = 0x30
    PIPE_AIRLOG = 0x31

class NestErrCode():
    NONE = 0
    INTERNAL = 1
    NO_MEM = 2
    NULL_POINTER = 3
    INVALID_ARGUMENT = 4
    NOT_READY = 5
    INVALID_STATE = 6
    NOT_FOUND = 7
    NOT_SUPPORT = 8
    TIMEOUT = 9
    OPERRATION_NOT_PERMITTED = 10
    IO_FAILED = 11
    BAD_ADDRESS = 12
    BUSY = 13
    INVALID_REQUEST_CODE = 14
    OPERATION_ALEARDY_IN_PROGRESS = 15
    VALUE_TOO_LARGE = 16
    LIMIT_OF_AVAILABLE_RESOURCE = 17
    DATA_SIZE_NOT_MATCH = 18
    FILE_NO_SUCH_FILE = 30
    FILE_BAD_FILE_NUMBER = 31
    FILE_APPEND_FAILED = 38
    FILE_INSUFFICIENT_STORAGE_CAPACITY = 39
    ELF_BAD_ELF_HEADER = 40
    ELF_NO_SYMBOL_TABLE = 41
    ELF_NO_STRING_TABLE = 42
    ELF_NO_TEXT_SEGMENT = 43
    ELF_SYMBOL_NOT_FOUND = 44
    ELF_SEGMENT_NOT_FOUND = 45
    ELF_NO_STARTING_POINT = 46

class NestCommand:
    def __init__(self, opcode, data):
        self.opcode = opcode
        self.data = data
        self.data_len = len(data)

    @property
    def data_len(self):
        return self._data_len

    @property
    def opcode(self):
        return self._opcode

    @opcode.setter
    def opcode(self, value):
        self._opcode = value

    @property
    def data(self):
        return self._data

    @data.setter
    def opcode(self, value):
        self._data = value

class NestSerialProcess:
    NEST_INTERFACE_TCP = 'TCP'
    NEST_INTERFACE_COM = 'COM'
    def __init__(self, interface, dest, callback=None):
        self.interface = interface
        self.abort = False
        self.rxstatus = 0
        self.rxlen = 0
        self.rxoffset = 0
        self.rxbuffer = bytearray(256)
        self.timer = None
        self.txseq = 0

        self.callback_func = callback

        if interface == NestSerialProcess.NEST_INTERFACE_TCP:
            self.server_address = (dest, 5897)
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect(self.server_address)
            thread = threading.Thread(target=self.sock_reader, args=(self.sock ,))
            thread.start()
        elif interface == NestSerialProcess.NEST_INTERFACE_COM:
            self.sp = serial.Serial(dest, 115200, timeout=1)
            thread = threading.Thread(target=self.serial_reader, args=(self.sp ,))
            thread.start()

    def parser_timeout(self):
        self.rxstatus = 0
        self.rxlen = 0
        self.rxoffset = 0
        self.rxbuffer = bytearray(256)

    def send(self, rawdata):
        if self.interface == NestSerialProcess.NEST_INTERFACE_TCP:
            hexstring = binascii.hexlify(rawdata)
            self.sock.send(hexstring + '\r\n')
        elif self.interface == NEST_INTERFACE_COM:
            if self.sp.isOpen() == True:
                packets = self.serial_pack(rawdata)
                self.sp.write(packets)

    def close(self):
        self.abort = True
        if self.interface == NestSerialProcess.NEST_INTERFACE_TCP:
            self.sock.close()
        elif self.interface == NestSerialProcess.NEST_INTERFACE_COM:
            self.sp.close()

    def readlines(self, recv_buffer=1024, delim='\n'):
    	buffer = ''
        try:
            while True:
                data = self.sock.recv(recv_buffer)
                buffer += data

                while buffer.find(delim) != -1:
                    line, buffer = buffer.split('\n', 1)
                    yield line
        except Exception as e:
            print 'tcp abort'
            return
    	return

    def sock_reader(self, sock):
        while not self.abort:
            for line in self.readlines():
                self.handle_payload(bytearray.fromhex(line.strip()))

    def handle_payload(self, payload):
        # print 'handle payload'
        if self.callback_func is not None:
            self.callback_func(payload)

    def serial_reader(self, ser):
        while not self.abort:
            num = ser.inWaiting()
            for i in range(0, num):
                pkt = ser.read()
                #print int(binascii.hexlify(pkt), 16)
                # print binascii.hexlify(pkt)
                pkt = int(binascii.hexlify(pkt), 16)
                # print 'rxstatus ' + (str(self.rxstatus))
                if self.rxstatus == 0:
                    if pkt == 0xA5:
                        self.rxstatus = 1
                        self.timer = threading.Timer(1.0, self.parser_timeout)
                elif self.rxstatus == 1:
                    if pkt == 0x5A:
                        self.rxstatus = 2
                    else:
                        self.rxstatus = 0
                elif self.rxstatus == 2:
                    self.rxlen = pkt
                    self.rxoffset = 0
                    self.rxstatus = 3
                    # print 'rxlen ' + str(self.rxlen)
                elif self.rxstatus == 3:
                    # print pkt
                    self.rxbuffer[self.rxoffset] = pkt
                    self.rxoffset+=1
                    if self.rxoffset == self.rxlen:
                        # print self.rxoffset
                        # print self.rxbuffer[self.rxoffset]
                        if self.rxbuffer[self.rxoffset-1] == 0xF5:
                            self.handle_payload(self.rxbuffer)
                            self.timer.cancel()
                            self.rxstatus = 0
                            self.rxlen = 0
                            self.rxoffset = 0
                            self.rxbuffer = bytearray(256)
                        else:
                            self.rxstatus = 0
                            self.rxlen = 0
                            self.rxoffset = 0
                            self.rxbuffer = bytearray(256)
                            self.timer.cancel()
            time.sleep(0.05)

    def command_pack(self, data, opcode):
        # total len 20
        header = bytearray(4)
        header[0] = NEST_SERIAL_PROTOCOL_VERSION << 5 | (self.txseq & 0x1F)
        header[1] = opcode
        content = bytearray(16)
        if data is None:
            header[2] = 0
            header[3] = 0
        else:
            header[2] = reduce(lambda i, j: int(i) ^ int(j), data)  # xor
            header[3] = len(data);
            for i in range(0, len(data)):
                content[i] = data[i]

        concat = header + content
        self.txseq+=1
        return concat

    def command_unpack(self, src):
        version = (src[0] & 0xE0) >> 5
        seq = (src[0] & 0x1F)
        opcode = src[1]
        len = src[3]
        if version != NEST_SERIAL_PROTOCOL_VERSION:
            return None
        content = bytearray(len)
        for i in range(0, len):
            content[i] = src[i+4]
        src_xor = reduce(lambda i, j: int(i) ^ int(j), content)
        if src_xor != src[2]:
            return None
        command = NestCommand(opcode, content)
        return command

    def serial_pack(self, src):
        prefix = bytearray(2)
        prefix[0] = 0xA5
        prefix[1] = 0x5A
        data_len = len(src) + 1
        prefix.append(data_len)
        dst = prefix + src
        dst.append(0xf5)
        return dst

class NestProcess(object):
    def __init__(self):
        self.abort = False
        signal.signal(signal.SIGINT, self.signal_handler)

    def signal_handler(self, signal, frame):
        # print 'NestProcess signal handler ctrl + c'
        self.abort = True
        self.abort_handler()

    def abort_handler(self):
        # print 'NestProcess abort_handler'
        pass

    def dispose(self):
        self.abort = True
        self.abort_handler()

    def wait(self):
        while self.abort is False:
            try:
                time.sleep(1)
            except:
                print 'application abort'
                quit()
