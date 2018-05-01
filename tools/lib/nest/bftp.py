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

from lib.projutil import ProjectUtil

class NestBftpInstallProcess(NestProcess):
    PREFIX_NAME = 0
    PREFIX_APP = 1
    PREFIX_FILE_TYPE = 2
    PREFIX_EXT_FILE = 3
    PREFIX_ICON = 4

    OPERATE_WRITE = 0
    OPERATE_READ = 1
    OPERATE_READDIR = 2

    def __init__(self, port, ipaddr, path, prefix):
        NestProcess.__init__(self)
        self.curr_path = path
        self.file_data = None
        self.data_size = 0
        self.offset = 0
        self.need_data_len = 0
        self.checksum = 0
        self.nsp = None
        self.operate_type = self.OPERATE_WRITE


        util = ProjectUtil()
        self.conf_content = util.load_conf(path, True)
        self.prefix_type = prefix
        self.short_uuid = self.conf_content['short_uuid']
        self.app_name = self.conf_content['name']

        # print self.app_name
        if self.prefix_type == self.PREFIX_APP:
            try:
                file_name = self.app_name + "-app.elf"
                app_file_path = os.path.join(path, file_name)
                file_bytes = open(app_file_path, 'rb').read()
                filebyte_array = bytearray(file_bytes)
                self.checksum = reduce(lambda i, j: int(i) ^ int(j), filebyte_array)
                self.data_size = len(file_bytes)
                self.file_data = filebyte_array
            except : # Guard against race condition
                print '{}-app.elf not found'.format(self.app_name)
                self.dispose()
        elif self.prefix_type == self.PREFIX_NAME:
            try:
                filebyte_array = bytearray(str(self.app_name))
                self.checksum = reduce(lambda i, j: int(i) ^ int(j), filebyte_array)
                self.data_size = len(filebyte_array)
                self.file_data = filebyte_array
            except : # Guard against race condition
                print 'failed to transfer the name of file'
                self.dispose()
        attr = bytearray(9)
        attr[0] = self.prefix_type
        attr[1] = int(self.short_uuid[0:2], 16)
        attr[3] = int(self.short_uuid[4:6], 16)
        attr[2] = int(self.short_uuid[2:4], 16)
        attr[4] = int(self.short_uuid[6:8], 16)
        attr[5] = (self.data_size & 0xFF000000) >> 24
        attr[6] = (self.data_size & 0x00FF0000) >> 16
        attr[7] = (self.data_size & 0x0000FF00) >> 8
        attr[8] = (self.data_size & 0x000000FF)

        if port is not '':
            self.nsp = NestSerialProcess(NestSerialProcess.NEST_INTERFACE_COM, port, self.data_received)
        elif ipaddr is not '':
            self.nsp = NestSerialProcess(NestSerialProcess.NEST_INTERFACE_TCP, ipaddr, self.data_received)

        # initiate init packetes
        command = self.nsp.command_pack(attr, NestOpcode.BFTP_INIT)
        self.nsp.send(command)

    def abort_with_error(self, err_code):
        if err_code == NestErrCode.FILE_INSUFFICIENT_STORAGE_CAPACITY:
            print 'Insufficient storage capacity'
        elif err_code == NestErrCode.FILE_NO_SUCH_FILE:
            print 'The file does not exist'
        elif err_code == NestErrCode.INVALID_ARGUMENT:
            print 'Invalid argument'
        elif err_code == NestErrCode.INVALID_REQUEST_CODE:
            print 'Invalid request'
        elif err_code == NestErrCode.NO_MEM:
            print 'No enough storage is available to process this command (Max App number is 9)'
        self.dispose()

    def data_received(self, data):
        command = self.nsp.command_unpack(data)
        # print vars(command)
        if not command == None:
            if command.opcode == (NestOpcode.BFTP_INIT | 0x80):
                # print 'Get the BFTP INIT Response'
                if command.data_len == 9:
                    #　Get the BFTP Init response, start to issue the first raw packets
                    if command.data[0] == 0x00:
                        ofs_arr = command.data[1:5]
                        len_arr = command.data[5:9]
                        self.offset = ofs_arr[0] | (ofs_arr[1] << 8) | (ofs_arr[2] << 16) | (ofs_arr[3] << 24)
                        self.need_data_len = len_arr[0] | (len_arr[1] << 8) | (len_arr[2] << 16) | (len_arr[3] << 24)
                        # print 'init response need len ' + str(self.need_data_len)
                        attr = self.file_data[0:self.need_data_len]
                        command = self.nsp.command_pack(attr, NestOpcode.BFTP_PACKETS)
                        # print binascii.hexlify(packets)
                        self.nsp.send(command)
                    else:
                        self.abort_with_error(command.data[0])
            elif command.opcode == (NestOpcode.BFTP_PACKETS | 0x80):
                # print 'Get the BFTP PACKETS Response'
                if command.data[0] != NestErrCode.NONE:
                    self.abort_with_error(command.data[0])
                else:
                    if command.data_len == 9:
                        ofs_arr = command.data[1:5]
                        len_arr = command.data[5:9]
                        self.offset = ofs_arr[0] | (ofs_arr[1] << 8) | (ofs_arr[2] << 16) | (ofs_arr[3] << 24)
                        self.need_data_len = len_arr[0] | (len_arr[1] << 8) | (len_arr[2] << 16) | (len_arr[3] << 24)
                        if self.need_data_len == 0:
                            attr = bytearray(4)
                            attr[0] = self.checksum
                            command = self.nsp.command_pack(attr, NestOpcode.BFTP_END)
                            self.nsp.send(command)
                        else:
                            attr = self.file_data[self.offset:self.offset+self.need_data_len]
                            command = self.nsp.command_pack(attr, NestOpcode.BFTP_PACKETS)
                            self.nsp.send(command)

                            # report progress
                            self.report_progress(self.offset, self.data_size, status='upload..')

            elif command.opcode == (NestOpcode.BFTP_END | 0x80):
                # show the complete progress
                if command.data[0] != NestErrCode.NONE:
                    self.abort_with_error(command.data[0])
                else:
                    if self.prefix_type == self.PREFIX_APP:
                        print 'install {}-app.elf to a/{}'.format(self.app_name, self.short_uuid)
                    elif self.prefix_type == self.PREFIX_NAME:
                        print 'setup the file name to n/{}'.format(self.short_uuid)
                    self.dispose()

    def report_progress(self, count, total, status=''):
        bar_len = 60
        filled_len = int(round(bar_len * count / float(total)))

        percents = round(100.0 * count / float(total), 1)
        bar = '=' * filled_len + '-' * (bar_len - filled_len)

        sys.stdout.write('[%s] %s%s ...%s\r' % (bar, percents, '%', status))
        sys.stdout.flush()

    def abort_handler(self):
        if self.nsp is not None:
            self.nsp.close()
        exit()
