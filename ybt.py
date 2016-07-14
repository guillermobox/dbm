#!/usr/bin/python2
import yaml
import subprocess
import threading
import os
import sys

def readlist(dict, key):
    if not key in dict:
        return []
    ret = dict[key]
    if type(ret) != list:
        ret = [ret]
    return ret

class Suite(object):
    def __init__(self, path):

        with open(path, 'r') as testfile:
            try:
                suite = yaml.load(testfile)
            except yaml.scanner.ScannerError as e:
                raise Exception("Input file is not YAML, error at line %d"%(e.problem_mark.line,))

        self.parseConfiguration(suite['configuration'])
        self.parseTests(suite['tests'])

    def parseConfiguration(self, config):
        self.environment = readlist(config, 'environment')
        self.volatile = readlist(config, 'volatile')

    def parseTests(self, configlist):
        self.tests = []
        for config in configlist:
            self.newTest(config)

    def newTest(self, configuration):
        self.tests.append(Test(configuration))

    def runTests(self):
        for test in self.tests:
            self.setup()
            err = test.run()
            test.show(err)
            self.setdown()

    def setup(self):
        self.savedEnvironment = dict()
        for pair in self.environment:
            key,_,value = pair.partition('=')
            if key in os.environ:
                self.savedEnvironment[key] = os.environ[key]
            os.environ[key] = value

        for path in self.volatile:
            try:
                os.remove(path)
            except:
                pass

    def setdown(self):
        for pair in self.environment:
            key,_,value = pair.partition('=')
            if key in self.savedEnvironment:
                os.environ[key] = self.savedEnvironment[key]
            else:
                del os.environ[key]

        for path in self.volatile:
            try:
                os.remove(path)
            except:
                pass

class Test(object):
    def __init__(self, config):
        self.name = config['name']
        self.command = config['run']
        self.setup = config['setup'] if 'setup' in config else []
        if type(self.setup) != list:
            self.setup = [self.setup]
        self.input = str(config['input']) if 'input' in config else None

        if 'output' in config:
            self.stdout = config['output'] or ""
        else:
            self.stdout = None

        self.retval = config['return'] if 'return' in config else None
        self.timeout = int(config['timeout']) if 'timeout' in config else 10

    def show(self, error):
        if error:
            print "{0:^20}\033[31mFAIL\033[0m {1}".format(self.name[0:20], error)
        else:
            print "{0:^20}\033[32m OK \033[0m".format(self.name[0:20])

    def run(self):
        for command in self.setup:
            if subprocess.call(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE):
                return "Setup failed"

        process = subprocess.Popen("exec " + self.command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)

        def target():
            (stdout, stderr) = process.communicate(self.input)
            self.runResults = (stdout, stderr)

        thread = threading.Thread(target=target)
        thread.start()

        thread.join(self.timeout)
        if thread.is_alive():
            process.kill()
            thread.join()
            return "Timeout of %d seconds"%(self.timeout,)

        if self.stdout != None and str(self.stdout).strip() != self.runResults[0]:
            return "Wrong output"

        if self.retval and self.retval != process.returncode:
            return "Wrong return value"

        return None

def main():
    suite = Suite(sys.argv[1])
    suite.runTests()

if __name__ == '__main__':
    exit(main())
