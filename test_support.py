# -*- coding: utf-8 -*-

import os
import time
import subprocess
import threading
import traceback
import shlex

import unittest
import test_globals

###################################################

def initTestSuite(testSuiteFilePath):
    test_globals.testSuiteFilePath = testSuiteFilePath

###################################################

def log(logstr):
    print("\n{0}".format(logstr))

def logDebug(logstr):
    if test_globals.debugMode:
        log(logstr)

###################################################

""" This class the the SST Unittest class """
class SSTUnitTest(unittest.TestCase):

    def __init__(self, methodName):
        super(SSTUnitTest, self).__init__(methodName)

###

    def log(self, logstr):
        log(logstr)

    def logDebug(self, logstr):
        logDebug(logstr)

###

    def isTestingInDebugMode(self):
        return test_globals.debugMode

    def getTestSuiteDir(self):
        return os.path.dirname(test_globals.testSuiteFilePath)

    def getTestOutputRunDir(self):
        return test_globals.testOutputRunDirPath

    def getTestOutputTmpDir(self):
        return test_globals.testOutputTmpDirPath

###

#    def shellCmd(command,



################################################################################

class Command(object):
    """
    Enables to run subprocess commands in a different thread with TIMEOUT option.
    Based on jcollado's solution:
    http://stackoverflow.com/questions/1191374/subprocess-with-timeout/4825933#4825933
    """
    command = None
    process = None
    status = None
    output, error = '', ''

    def __init__(self, command):
        if isinstance(command, basestring):
            command = shlex.split(command)
        self.command = command

    def run(self, timeout=None, **kwargs):
        """ Run a command then return: (status, output, error). """
        def target(**kwargs):
            try:
                self.process = subprocess.Popen(self.command, **kwargs)
                self.output, self.error = self.process.communicate()
                self.status = self.process.returncode
            except:
                self.error = traceback.format_exc()
                self.status = -1

        # default stdout and stderr
        if 'stdout' not in kwargs:
            kwargs['stdout'] = subprocess.PIPE
        if 'stderr' not in kwargs:
            kwargs['stderr'] = subprocess.PIPE

        # thread
        thread = threading.Thread(target=target, kwargs=kwargs)
        thread.start()
        thread.join(timeout)
        if thread.is_alive():
            self.process.terminate()
            thread.join()
        return self.status, self.output, self.error

################################################################################

class Command1(object):
    """
    Enables to run subprocess commands in a different thread with TIMEOUT option.
    Based on jcollado's solution:
    http://stackoverflow.com/questions/1191374/subprocess-with-timeout/4825933#4825933
    """
    outputfilepath = None
    command = None
    process = None
    status  = None
    output  = ''
    error   = ''

    def __init__(self, command, outputfilepath=None):
        if isinstance(command, basestring):
            command = shlex.split(command)
        self.command = command
        self.outputfilepath = outputfilepath

    def run(self, timeout=None, **kwargs):
        """ Run a command then return: (status, output, error). """
        def target(**kwargs):
            try:
                if self.outputfilepath != None:
                    with open(self.outputfilepath, 'w+') as f:
                        self.process = subprocess.Popen(self.command, stdout=f, **kwargs)
                        self.output, self.error = self.process.communicate()
                        self.status = self.process.returncode
                else:
                        self.process = subprocess.Popen(self.command, **kwargs)
                        self.output, self.error = self.process.communicate()
                        self.status = self.process.returncode
            except:
                self.error = traceback.format_exc()
                self.status = -1

        # See if we need to redirect
#        if self.outputfilepath != None:
#            with open(self.outputfilepath, 'w+') as f:
#                kwargs['stdout'] = f

        # default stdout and stderr
#        if 'stdout' not in kwargs:
#            kwargs['stdout'] = subprocess.PIPE
#        if 'stderr' not in kwargs:
#            kwargs['stderr'] = subprocess.PIPE

        # thread
        thread = threading.Thread(target=target, kwargs=kwargs)
        thread.start()
        thread.join(timeout)
        if thread.is_alive():
            self.process.terminate()
            thread.join()
        return self.status, self.output, self.error




