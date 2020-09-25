#! /usr/bin/env python

import json
import sys
import os

from argparse import ArgumentParser

parser = ArgumentParser(description='Check if workers or job processes are running')
parser.add_argument('--config', default='config.json', action='store', help='Your system config (in JSON)')
parser.add_argument('--job', default='job.json', action='store', help='Your config (in JSON) for submitting job')
parser.add_argument('--jm', action='store_true', help='check jms')
parser.add_argument('-w','--check-workers', default=1, type=int, action='store', help='Check workers')
parser.add_argument('-j','--job-id', type=int, action='store', help='Check job processes')

# Select pssh command
pssh = ''
if os.popen('which pssh').read().strip() != '':
    pssh = 'pssh'
elif os.popen('which parallel-ssh').read().strip() != '':
    pssh = 'parallel-ssh'
else:
    sys.stderr.write("Cannot find command pssh or parallel-ssh.\n")
    exit(0)

def run(cmd):
    #print(cmd)
    #print(os.popen(cmd).read().strip())
    os.system(cmd)

def check_jps(config, job_conf, job_id):
    jp_name = job_conf["job_process"].rsplit('/', 1)[1]
    host_opt = '-H ' + ' -H '.join(config['workers'])
    print("Running jps: ")
    cmd = '{3} -P {0} "ps x -o pid,command | grep -E \'JP-[0-9.]*\.{2} \' -o | grep -v grep" | grep -vE "SUCCESS|FAILURE" | sort -V '.format(host_opt, jp_name, job_id, pssh)
    run(cmd)

def check_jms(config, job_conf, job_id):
    jm_name = job_conf["job_manager"].rsplit('/', 1)[1]
    host_opt = '-H ' + ' -H '.join(config['workers'])
    if job_id is not None:
        print("Running jm: ")
        cmd = '{3} -P {0} "ps x -o pid,command | grep -E \'{1}-[0-9.]*\.{2} \' -o | grep -v grep" | grep -vE "SUCCESS|FAILURE" | sort -V '.format(host_opt, jm_name, job_id, pssh)
    else:
        print("Running jms: ")
        cmd = '{3} -P {0} "ps x -o pid,command | grep -v grep | grep -E \'{1}[^\s]* \' -o " | grep -vE "SUCCESS|FAILURE" | sort -V '.format(host_opt, jm_name, job_id, pssh)
    run(cmd)

def check_workers(config, job_conf):
    host_opt = '-H ' + ' -H '.join(config['workers'])
    check_jp_cmd = '{2} -P {0} "pid=\`cat {1}\` && kill -0 \$pid && echo -e \\"\e[32mWorker is running\e[0m\\" || echo -e \\"\e[31mWorker is not running\e[0m\\"" | grep --color=always -v "[SUCCESS]" | sort -V'.format(
            host_opt,
            os.path.join(config['worker_husky_scratch_dir'], "Worker.*.pid"),
            pssh
        )
    run(check_jp_cmd)


if __name__ == '__main__':
    args = parser.parse_args(sys.argv[1:])
    config = json.load(open(args.config))
    job_conf = json.load(open(args.job))

    if args.check_workers:
        # TODO: check master and scheduler
        check_workers(config, job_conf)

    if args.job_id != None:
        check_jps(config, job_conf, args.job_id)

    if args.jm:
        check_jms(config, job_conf, args.job_id)
