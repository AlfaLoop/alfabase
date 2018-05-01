# -*- coding: utf-8 -*-
import serial
import argparse
import sys
import os
import struct
import json
import threading
import signal

def install_cmd_handler(port):
    print 'install process ' + port

def ts_cmd_handler(port):
    print 'timesync process ' + port

def run_cmd_handler(port):
    print 'run process ' + port

def kill_cmd_handler(port):
    print 'kill process ' + port

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers()

    parser_install = subparsers.add_parser('install', help='Install application to target board')
    parser_install.add_argument("-p", "--port", help="Serial driver (e.g COM1, ttyUSB0, ttyS0)", dest="port", default="")
    parser_install.set_defaults(func=install_cmd_handler, newstate='install')

    parser_ts = subparsers.add_parser('timesync', help='Synchronize the timestamp of target')
    parser_ts.add_argument("-p", "--port", help="Serial driver (e.g COM1, ttyUSB0, ttyS0)", dest="port", default="")
    parser_ts.set_defaults(func=ts_cmd_handler, newstate='ts')

    parser_run = subparsers.add_parser('run', help='Execute application on edge device')
    parser_run.add_argument("-p", "--port", help="Serial driver (e.g COM1, ttyUSB0, ttyS0)", dest="port", default="")
    parser_run.set_defaults(func=run_cmd_handler, newstate='ts')

    parser_kill = subparsers.add_parser('kill', help='Kill application on edge device')
    parser_kill.add_argument("-p", "--port", help="Serial driver (e.g COM1, ttyUSB0, ttyS0)", dest="port", default="")
    parser_kill.set_defaults(func=kill_cmd_handler, newstate='ts')

    #parser.print_help()
    args = parser.parse_args()
    print vars(args)
    args.func(args.port)
