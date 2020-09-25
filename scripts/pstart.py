#! /usr/bin/env python

import os

from argparse import ArgumentParser
from json import load
from sys import stderr
from sys import stdout
from time import sleep

master_prog_gflags = [
    "master_husky_scratch_dir",
    "client_listener_port",
    "master_hostname",
    "master_port",
    "hdfs_namenode",
    "hdfs_port",
]
worker_prog_gflags = [
    "master_hostname",
    "master_port",
    "worker_husky_scratch_dir",
    "hdfs_namenode",
    "hdfs_port"
]

# Select pssh & pscp command
pssh = ''
pscp = ''
if os.popen('which pssh').read().strip() != '':
    pssh = 'pssh'
    pscp = 'pscp'
elif os.popen('which parallel-ssh').read().strip() != '':
    pssh = 'parallel-ssh'
    pscp = 'parallel-scp'
else:
    sys.stderr.write("Cannot find command pssh or parallel-ssh.\n")
    exit(0)


def parse_cmd(argv=None):
    parser = ArgumentParser(description='Start system. Start workers in parallel.')
    parser.add_argument('-C', '--config', dest='config', default='config.json', help='Your config (in JSON) for running AXE.')
    parser.add_argument('--container', dest='container', action='store_true', help='Launch container-based version.')
    return parser.parse_args(argv)


def get_flags(config, prog_gflags):
    required = ' '.join(['--{0}={1}'.format(f, config[f]) for f in prog_gflags])
    advanced = ' '.join(['--{0}={1}'.format(key, val) for key, val in config["advanced"].items()])
    return '{0} {1}'.format(required, advanced)


def start_master(config):

    # Check whether there's already a running Master
    check_pid_cmd = 'ssh {0} "mkdir -p {1} && cd {1} && cat Master.pid 2> /dev/null"'.format(
            config['master_hostname'], config['master_husky_scratch_dir']
        )
    existing_pid = os.popen(check_pid_cmd).read().strip()
    if existing_pid != "":
        stdout.write("Master already started (PID: %s)\n" % existing_pid)
        return

    # SCP Master binary
    master_bin_file = os.path.join(config['master_husky_bin_dir'], 'Master')
    scp_cmd = 'scp {0} {1}:{2}'.format(
            master_bin_file, config['master_hostname'], config['master_husky_scratch_dir']
            )
    scp_status = os.popen(scp_cmd).read().strip()
    if scp_status != "":
        stderr.write(scp_status)
        stderr.write("\n")

    # Start Master
    start_master_cmd = 'ssh {0} "cd {1} && bash --login -c \'./Master {2} --log_dir={1} >{3} 2>{4} & echo \$! > Master.pid\'" && echo Master started'.format(
        config['master_hostname'],
        config['master_husky_scratch_dir'],
        get_flags(config, master_prog_gflags),
        os.path.join(config['master_husky_scratch_dir'], 'stdout'),
        os.path.join(config['master_husky_scratch_dir'], 'stderr'))

    print(os.popen(start_master_cmd).read().strip())


def start_workers(config, args):
    # Check running Worker(s)
    tmp_dir = '~/tmp/axe_host'
    host_list = []
    host_opt = '-H ' + ' -H '.join(config['workers'])
    check_pid_cmd = 'mkdir -p {2} && {3} {0} -o {2} "mkdir -p {1} && cd {1} && cat Worker.*.pid 2> /dev/null"'.format(
                host_opt, config['worker_husky_scratch_dir'], tmp_dir, pssh
            )
    existing_pid = os.popen(check_pid_cmd).read().strip()

    for w in config['workers']:
        pid = open(os.path.expanduser(os.path.join(tmp_dir,w))).read().strip();
        if pid != "":
            stdout.write("Worker on {0} already started (PID: {1})\n".format(w, pid))
        else:
            host_list.append(w)
    os.system('rm {0} -r'.format(tmp_dir))

    if len(host_list) is 0:
        return
    host_opt = '-H ' + ' -H '.join(host_list)

    # SCP Worker binary
    stdout.write("scp file to hosts: {0}\n".format(' '.join(host_list)))
    worker_bin_file = os.path.join(config['worker_husky_bin_dir'], 'Worker')
    os.popen('{3} {0} {1} {2}'.format(host_opt, worker_bin_file, config['worker_husky_scratch_dir'], pscp)).read()

    # Start Worker
    stdout.write("Starting workers on hosts: {0}\n".format(' '.join(host_list)))
    local_host_name = '\`hostname\`'
    start_worker_cmd = '{0} --inline {1} -t 10 "cd {2} && bash --login -c \'./Worker {3} --log_dir={2} >{4} 2>{5} & echo \$! > Worker.{6}.pid\' "'.format(
      pssh,
      host_opt,
      config['worker_husky_scratch_dir'],
      get_flags(config, worker_prog_gflags),
      os.path.join(config['worker_husky_scratch_dir'], 'stdout.{}'.format(local_host_name)),
      os.path.join(config['worker_husky_scratch_dir'], 'stderr.{}'.format(local_host_name)),
      local_host_name,
    )

    os.system(start_worker_cmd)


def start_scheduler(config_file, container_version):
    scheduler_script = os.path.join(os.path.dirname(__file__), "scheduler.py")
    if container_version:
        os.system("{0} --start=1 --config {1} --container".format(scheduler_script, config_file))
    else:
        os.system("{0} --start=1 --config {1}".format(scheduler_script, config_file))


if __name__ == '__main__':
    args = parse_cmd()
    config = load(open(args.config))

    start_master(config)
    start_workers(config, args)
    sleep(2); # TODO(tatiana): support dynamic worker config in scheduler
    start_scheduler(args.config, args.container)
