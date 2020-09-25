#! /usr/bin/env python

import os
import sys

import json

from argparse import ArgumentParser

prog_gflags = [
    "master_hostname",
    "master_port",
]

def parse_cmd(argv=None):
    parser = ArgumentParser(description='Start/Stop scheduler.')
    parser.add_argument('-C', '--config', dest='config', default='config.json', help='Your config (in JSON) for running AXE.')
    parser.add_argument('--start', dest='start', type=int, help='Start Scheduler if true, else stop Scheduler.')
    parser.add_argument('--container', dest='container', action='store_true', help='Launch container-based version.')
    return parser.parse_args(argv)


def get_flags(config):
    required = ' '.join(['--{0}={1}'.format(f, config[f]) for f in prog_gflags])
    advanced = ' '.join(['--{0}={1}'.format(key, val) for key, val in config["advanced"].items()])
    return '{0} {1}'.format(required, advanced)


def get_existing_pid():
    check_existing_scheduler_cmd = 'ssh {0} "cat {1} 2> /dev/null"'.format(
      config['master_hostname'], os.path.join(config['master_husky_scratch_dir'], "Scheduler.pid"))
    return os.popen(check_existing_scheduler_cmd).read().strip()


# TODO(tatiana): launch only one scheduler on the master machine for now
def start_scheduler(config, container_version):
    # Check whether there's already a running Scheduler
    existing_pid = get_existing_pid()
    if existing_pid != "":
        sys.stdout.write("Scheduler already started (PID: {0})\n".format(existing_pid))
        return

    scheduler_path = os.path.join(config['master_husky_bin_dir'], 'ContainerScheduler' if container_version else 'Scheduler')
    # Start Scheduler
    start_scheduler_cmd = 'ssh {0} "cd {1} && bash --login -c \'{2} {3} --log_dir={1} >{4} 2>{5} & echo \$! > Scheduler.pid\'"&'.format(
      config['master_hostname'],
      config['master_husky_scratch_dir'],
      scheduler_path,
      get_flags(config),
      os.path.join(config['master_husky_scratch_dir'], 'Scheduler.stdout'),
      os.path.join(config['master_husky_scratch_dir'], 'Scheduler.stderr'))

    os.system(start_scheduler_cmd)
    sys.stdout.write("Scheduler started.\n")


def stop_scheduler(config):
    # Check whether there's a running Scheduler
    existing_pid = get_existing_pid()
    if existing_pid == "":
        sys.stdout.write("Scheduler is not running\n")
        return

    # Stop Scheduler
    stop_scheduler_cmd = 'ssh {0} "kill {1}; res=\`ps -o pid= -p {1}\` && if [ -z \$res ]; then rm {2}; fi"'.format(
      config['master_hostname'], existing_pid, os.path.join(config['master_husky_scratch_dir'], 'Scheduler.pid'))
    os.system(stop_scheduler_cmd)
    sys.stdout.write("Scheduler stopped.\n")


if __name__ == '__main__':
    args = parse_cmd()
    config = json.load(open(args.config))
    if (args.start):
        start_scheduler(config, args.container)
    else:
        stop_scheduler(config)
