# -*- coding: utf-8 -*-

import os

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
