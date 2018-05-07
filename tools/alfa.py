# -*- coding: utf-8 -*-
import serial
import argparse
import sys
import os
import struct
import json
import threading
import signal

from lib.projutil import ProjectCmd
from lib.nest.bftp import NestBftpInstallProcess
from lib.nest.timesync import NestTimeSyncProcess
from lib.nest.execute import NestExecuteProcess
from lib.nest.remove import NestRemoveProcess
from lib.nest.terminate import NestTerminateProcess
from lib.nest.boot import NestSetBootProcess
from lib.nest.boot import NestDelBootProcess

def abort_and_show_help(parser):
    parser.print_help()
    quit()

def proj_cmd_handler(cmd):
    ProjectCmd(cmd, os.getcwd())

def install_cmd_handler(port, ipaddr):
    # app
    process = NestBftpInstallProcess(port, ipaddr, os.getcwd(), NestBftpInstallProcess.PREFIX_APP)
    process.wait()
    process = NestBftpInstallProcess(port, ipaddr, os.getcwd(), NestBftpInstallProcess.PREFIX_NAME)
    process.wait()

def remove_cmd_handler(port, ipaddr):
    process = NestRemoveProcess(port, ipaddr, os.getcwd())
    process.wait()

def run_cmd_handler(port, ipaddr):
    process = NestExecuteProcess(port, ipaddr, os.getcwd())
    process.wait()

def kill_cmd_handler(port, ipaddr):
    process = NestTerminateProcess(port, ipaddr, os.getcwd())
    process.wait()

def ts_cmd_handler(port, ipaddr):
    process = NestTimeSyncProcess(port, ipaddr, os.getcwd())
    process.wait()

def list_cmd_handler(port, ipaddr):
    pass

def setboot_cmd_handler(port, ipaddr):
    process = NestSetBootProcess(port, ipaddr, os.getcwd())
    process.wait()

def delboot_cmd_handler(port, ipaddr):
    process = NestDelBootProcess(port, ipaddr, os.getcwd())
    process.wait()

def airlog_cmd_handler(port, ipaddr):
    pass

def version_cmd_handler():
    print 'Version 0.0.1'

if __name__ == "__main__":
    # alfa.py -c [deploy, remove, list, get]
    # alfa.py -c [run, kill, query]
    # alfa.py -c [log, pipe]
    # alfa.py -c [power, reboot]
    # alfa.py -c [build, clean, new]  (O)
    parser = argparse.ArgumentParser(description='alfa command-line tools helps' +
                                    ' you to create your own app on AlfaOS.' +
                                    ' For more information on the AlfaOS, please visit https://www.alfaloop.com.')
    subparsers = parser.add_subparsers()

    # application builder
    parser_build = subparsers.add_parser('build', help='Compiles the project (wrap the GCC tools)')
    parser_build.set_defaults(func=proj_cmd_handler, newstate='build')

    parser_new = subparsers.add_parser('new', help='Create a new alfabase app')
    parser_new.set_defaults(func=proj_cmd_handler, newstate='new')

    parser_clean = subparsers.add_parser('clean', help='Deletes build artifacts in the current project')
    parser_clean.set_defaults(func=proj_cmd_handler, newstate='clean')

    # nest protocol
    parser_install = subparsers.add_parser('install', help='Install applications on the target board')
    parser_install.add_argument("-p", "--port", help="Serial driver (e.g COM1, ttyUSB0, ttyS0)", dest="port", default="")
    parser_install.add_argument("-i", "--ip", help="IP Address (e.g 192.168.1.1)", dest="ipaddr", default="")
    parser_install.set_defaults(func=install_cmd_handler, newstate='install')

    parser_remove = subparsers.add_parser('remove', help='Remove application on the target board')
    parser_remove.add_argument("-p", "--port", help="Serial driver (e.g COM1, ttyUSB0, ttyS0)", dest="port", default="")
    parser_remove.add_argument("-i", "--ip", help="IP Address (e.g 192.168.1.1)", dest="ipaddr", default="")
    parser_remove.set_defaults(func=remove_cmd_handler, newstate='remove')

    parser_run = subparsers.add_parser('run', help='Execute application on the target board')
    parser_run.add_argument("-p", "--port", help="Serial driver (e.g COM1, ttyUSB0, ttyS0)", dest="port", default="")
    parser_run.add_argument("-i", "--ip", help="IP Address (e.g 192.168.1.1)", dest="ipaddr", default="")
    parser_run.set_defaults(func=run_cmd_handler, newstate='run')

    parser_kill = subparsers.add_parser('kill', help='Kill application on the target board')
    parser_kill.add_argument("-p", "--port", help="Serial driver (e.g COM1, ttyUSB0, ttyS0)", dest="port", default="")
    parser_kill.add_argument("-i", "--ip", help="IP Address (e.g 192.168.1.1)", dest="ipaddr", default="")
    parser_kill.set_defaults(func=kill_cmd_handler, newstate='kill')

    parser_ts = subparsers.add_parser('ts', help='Synchronize the time on the target board')
    parser_ts.add_argument("-p", "--port", help="Serial driver (e.g COM1, ttyUSB0, ttyS0)", dest="port", default="")
    parser_ts.add_argument("-i", "--ip", help="IP Address (e.g 192.168.1.1)", dest="ipaddr", default="")
    parser_ts.set_defaults(func=ts_cmd_handler, newstate='ts')

    parser_listfile = subparsers.add_parser('list', help='List the files installed on the target board')
    parser_listfile.add_argument("-p", "--port", help="Serial driver (e.g COM1, ttyUSB0, ttyS0)", dest="port", default="")
    parser_listfile.add_argument("-i", "--ip", help="IP Address (e.g 192.168.1.1)", dest="ipaddr", default="")
    parser_listfile.set_defaults(func=list_cmd_handler, newstate='list')

    parser_setboot = subparsers.add_parser('setboot', help='Application that will be executed whenever the board is powered up or reset')
    parser_setboot.add_argument("-p", "--port", help="Serial driver (e.g COM1, ttyUSB0, ttyS0)", dest="port", default="")
    parser_setboot.add_argument("-i", "--ip", help="IP Address (e.g 192.168.1.1)", dest="ipaddr", default="")
    parser_setboot.set_defaults(func=setboot_cmd_handler, newstate='setboot')

    parser_delboot = subparsers.add_parser('delboot', help='Delete the boot application')
    parser_delboot.add_argument("-p", "--port", help="Serial driver (e.g COM1, ttyUSB0, ttyS0)", dest="port", default="")
    parser_delboot.add_argument("-i", "--ip", help="IP Address (e.g 192.168.1.1)", dest="ipaddr", default="")
    parser_delboot.set_defaults(func=delboot_cmd_handler, newstate='delboot')

    parser_airlog = subparsers.add_parser('airlog', help='Listen air log message on the target board')
    parser_airlog.add_argument("-p", "--port", help="Serial driver (e.g COM1, ttyUSB0, ttyS0)", dest="port", default="")
    parser_airlog.add_argument("-i", "--ip", help="IP Address (e.g 192.168.1.1)", dest="ipaddr", default="")
    parser_airlog.set_defaults(func=airlog_cmd_handler, newstate='airlog')

    parser_version = subparsers.add_parser('version', help='Display the current version')
    parser_version.set_defaults(func=version_cmd_handler, newstate='version')

    args = parser.parse_args()
    # print vars(args)
    spcommands = ['install', 'remove', 'ts', 'run', 'kill', 'list', 'setboot', 'delboot', 'log']
    projcommands = ['new', 'build', 'clean']
    othercommands = ['version']
    if args.newstate in spcommands:
        if args.port == '' and args.ipaddr == '':
            abort_and_show_help(parser)
        else:
            args.func(args.port, args.ipaddr)
    elif args.newstate in projcommands:
        args.func(args.newstate)
    elif args.newstate in othercommands:
        args.func()
    else:
        abort_and_show_help(parser)
