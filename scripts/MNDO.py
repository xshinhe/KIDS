import argparse
import datetime
import math
import os
import re
import shutil
import subprocess
import sys
import time
import toml
from copy import deepcopy
from multiprocessing import Pool
from pprint import pprint
from traceback import format_exc
from socket import gethostname
import numpy as np

parser = argparse.ArgumentParser(description='Execute MNDO Calculation')
parser.add_argument('integers', metavar='N', type=int, nargs='+', 
    help='an integer for the accumulator')
parser.add_argument('--sum', dest='accumulate', action='store_const', const=sum, default=max,
    help='sum the integers (default: find the max)')


def qm_job(qm_data):
    qm_config = qm_data["qm_config"]
    mndo_config = qm_config["QM"]["MNDO"]
    natom = qm_data["natom"]
    znumber = qm_data["znumber"]
    xyz = qm_data["geom_xyz"]
    
    job_str = ""
    if "toplines" in mndo_config:
        job_str += mndo_config["toplines"]  + '\n'
    elif "first_keywords" in mndo_config:
        for k in mndo_config["first_keywords"]:
            if mndo_config["first_keywords"][k] == True:
                job_str += mndo_config["first_keywords"][k] + ' '
            else:
                job_str += k + '=' + mndo_config["first_keywords"][k] + ' '
        job_str += '+\n'

        if "second_keywords" not in mndo_config:
            print('error')
            exit(0)

        for k in mndo_config["second_keywords"]:
            if mndo_config["second_keywords"][k] == True:
                job_str += mndo_config["second_keywords"][k] + ' '
            else:
                job_str += k + '=' + mndo_config["second_keywords"][k] + ' '
        job_str += '\n'
    elif "keywords" in mndo_config:
        for k in mndo_config["keywords"]:
            if mndo_config["keywords"][k] == True:
                job_str += mndo_config["keywords"][k] + ' '
            else:
                job_str += k + '=' + mndo_config["keywords"][k] + ' '
        job_str += '\n'

    job_str += "Automatically generated by MNDO.py \n[%s]\n" % datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')

    for i in range(natom):
        job_str += f"{znumber[i]} {xyz[i][1]} 0 {xyz[i][2]} 0 {xyz[i][3]} 0\n"
    job_str += "0 0 0 0 0 0 0\n"

    if "bottomlines" in mndo_config:
        job_str += mndo_config["bottomlines"]  + '\n'

    print(job_str)


def main():
    return

if __name__ == '__main__':
    main()