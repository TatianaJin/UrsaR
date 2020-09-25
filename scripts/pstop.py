#! /usr/bin/env python

import os

from argparse import ArgumentParser
from json import load
from sys import stderr
from sys import stdout

def select_pssh():
    """ Select pssh command """
    if os.popen('which pssh').read().strip() != '':
        return 'pssh'
    elif os.popen('which parallel-ssh').read().strip() != '':
        return 'parallel-ssh'
    else:
        sys.stderr.write("Cannot find command pssh or parallel-ssh.\n")
        exit(0)


def parse_cmd(argv=None):
    parser = ArgumentParser(description='Start system. Start workers in parallel.')
    parser.add_argument('-C', '--config', dest='config', default='config.json', help='Your config (in JSON) for running AXE.')
    return parser.parse_args(argv)


def stop_master(config):

    # Check whether there's already a running Master
    check_pid_cmd = 'ssh {0} "cd {1} 2>/dev/null && cat Master.pid 2> /dev/null"'.format(
            config['master_hostname'], config['master_husky_scratch_dir']
        )
    existing_pid = os.popen(check_pid_cmd).read().strip()
    if existing_pid == "":
        stdout.write("Master is not running\n")
        return

    # Stop Master and clear up
    stop_master_cmd = 'ssh {0} "kill {1} && echo Master stopped; rm -rf {2}"'.format(config['master_hostname'], existing_pid, config['master_husky_scratch_dir'])
    stdout.write(os.popen(stop_master_cmd).read())


def stop_workers(config, pssh):
    host_opt = '-H ' + ' -H '.join(config['workers'])
    stop_workers_cmd = '{3} {0} -P -t 10 "pid=\`cat {1}\` && pkill -P \$pid; kill -9 \$pid; rm -rf {2}"'.format(
            host_opt,
            os.path.join(config['worker_husky_scratch_dir'], "Worker.*.pid"),
            config['worker_husky_scratch_dir'], pssh
            )
    os.system(stop_workers_cmd)


def stop_scheduler(config_file):
    os.system("{0} --start=0 --config {1}".format(
      os.path.join(os.path.dirname(__file__), "scheduler.py"), config_file))


if __name__ == '__main__':
    args = parse_cmd()
    config = load(open(args.config))

    stop_scheduler(args.config)
    stop_master(config)
    stop_workers(config, select_pssh())
