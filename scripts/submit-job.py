#! /usr/bin/env python3

import os
import struct
import sys
import socket

import json
import zmq

from argparse import ArgumentParser
from datetime import datetime
from time import time

MasterNewJobEventType = 1


def to_datetime(milliseconds_since_epoch):
    return datetime.fromtimestamp(milliseconds_since_epoch / 1000.0).strftime('%Y-%m-%d %H:%M:%S.%f')

def pack(data, mode='int'):
    if mode == 'int':
        return struct.pack('i', data)
    elif mode == 'str':
        data = str(data).encode('utf-8')
        return struct.pack('{0}s'.format(len(data)), data)
    elif mode == 'dummy':
        return struct.pack('x', '')


def copy_job_to_master(job, config):
    """ Overwrite the job for each submission.  """
    job_path = os.path.abspath(job)
    if not os.path.isfile(job_path):
        sys.exit("{} is not a file!".format(job))

    local_name = os.path.basename(job_path)+ "-"  + str(int(time()))
    copy_job_to_master_cmd = 'scp -q {0} {1}:{2}/{3}'.format(
            job_path, config['master_hostname'],
            config['master_husky_scratch_dir'], local_name)
    os.system(copy_job_to_master_cmd)
    return local_name

def available_port():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind(('localhost', 0))
    addr, port = s.getsockname()
    s.close()
    return port

def submit_job(config, report):
    jm_file_name = copy_job_to_master(config['job_manager'], config)
    jp_file_name = copy_job_to_master(config['job_process'], config)
    config_file_name = copy_job_to_master(config['config_file'], config) if 'config_file' in config else ""

    context = zmq.Context()
    send_sock = zmq.Socket(context, zmq.PUSH)
    send_url = "tcp://{0}:{1}".format(config['master_hostname'], config['client_listener_port'])

    sys.stdout.write("Connecting to Master at {}\n".format(send_url))
    send_sock.connect(send_url)

    if report:
        recv_sock = zmq.Socket(context, zmq.PULL)
        port = available_port()
        recv_url = "tcp://*:{}".format(port)
        sys.stdout.write("Binding to local host at {}\n".format(recv_url))
        recv_sock.bind(recv_url)

    send_sock.send(pack(MasterNewJobEventType))
    send_sock.send(pack(jm_file_name, mode='str'))
    send_sock.send(pack(jp_file_name, mode='str'))
    send_sock.send(pack(config_file_name, mode='str'))
    if report:
        send_sock.send(pack(1, mode='int'))
        send_sock.send(pack(socket.gethostname(), mode='str'))
        send_sock.send(pack(port, mode='str'))
    else :
        send_sock.send(pack(0, mode='int'))
    # sock.send(pack(int(shutdown_after_finish)))
    print("Submit Job {0} & {1} Done".format(config['job_manager'], config['job_process']))

    if report:
        start_time = int(recv_sock.recv().decode('utf8'))
        print("Job starts at {}".format(to_datetime(start_time)))
        finish_time = int(recv_sock.recv().decode('utf8'))
        print("Job finishes at {0}, used {1} ms".format(to_datetime(finish_time), finish_time - start_time))

def parse_cmd(argv=None):
    parser = ArgumentParser(description='Submit job to the system.')
    parser.add_argument('--config', default='job.json',
                        help='Your job config (in JSON). Default is job.json.')
    parser.add_argument('-j', '--job-conf', help='The job config file.')
    parser.add_argument('-jm', '--job-manager', help='The binary file of the job manager executable.')
    parser.add_argument('-jp', '--job-process', help='The binary file of the job process executable.')
    parser.add_argument('-m', '--master-host', help='The hostname of the master.')
    parser.add_argument('-p', '--port', help='The client listener port on master host.')
    parser.add_argument('-d', '--scratch-dir', help='Master scratch dir.')
    parser.add_argument('-v', '--verbose', action='store_true', help='Print config.')
    parser.add_argument('-t', '--time', action='store_true', help='Report the job start time and job finish time.')
    return parser.parse_args(argv)

def get_config():
    args = parse_cmd()
    config = json.load(open(args.config))

    # overwrite with user specified value
    config["master_hostname"] = config["master_hostname"] if args.master_host is None else args.master_host
    config["client_listener_port"] = config["client_listener_port"] if args.port is None else args.port
    config["master_husky_scratch_dir"] = config["master_husky_scratch_dir"] if args.scratch_dir is None else args.scratch_dir
    config["job_manager"] = config["job_manager"] if args.job_manager is None else args.job_manager
    config["job_process"] = config["job_process"] if args.job_process is None else args.job_process
    config["config_file"] = config["config_file"] if args.job_conf is None else args.job_conf

    report = False
    if args.verbose:
        print_config(config)
    if args.time:
        report = True
    return config, report;

def print_config(config):
    print("master_hostname\t\t={0}".format(config["master_hostname"]))
    print("client_listener_port\t={0}".format(config["client_listener_port"]))
    print("master_husky_scratch_dir={0}".format(config["master_husky_scratch_dir"]))
    print("job_manager\t\t={0}".format(config["job_manager"]))
    print("job_process\t\t={0}".format(config["job_process"]))
    print("config_file\t\t={0}".format(config["config_file"]))


if __name__ == "__main__":
    config, report = get_config()
    submit_job(config, report)
