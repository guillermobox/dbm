#!/usr/bin/python2
import yaml
import subprocess
import os
import sys

def run(name, config):
    if 'setup' in config:
        subprocess.call(config['setup'], shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    process = subprocess.Popen(config['run'], shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    (stdout, stderr) = process.communicate()
    process.wait()

    if 'output' in config:
        if str(config['output']) != stdout:
            return "Wrong output"

    if 'error' in config:
        if config['error'] != process.returncode:
            return "Wrong return value"

    return None

def setup():
    os.environ["DBM_PATH"] = "/tmp/test.db"
    os.remove("/tmp/test.db")

def setdown():
    os.remove("/tmp/test.db")

def show(error, name, config):
    if error:
        print "{0:^20}\033[31mFAIL\033[0m {1}".format(name[0:20], error)
    else:
        print "{0:^20}\033[32m OK \033[0m".format(name[0:20])

if __name__ == '__main__':
    with open(sys.argv[1], 'r') as testfile:
        tests = yaml.load(testfile)
        for name, config in tests.iteritems():
            setup()
            err = run(name, config)
            show(err, name, config)
            if err:
                break
    setdown()


