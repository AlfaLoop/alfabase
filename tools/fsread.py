# -*- coding: utf-8 -*-
import subprocess
import sys
import os
import time


if __name__=='__main__':
    process = subprocess.check_output(["nrfjprog", "--memrd", "0x6F000", "--n", "0x10000", "-f", "nrf52"]).splitlines()
    print process
