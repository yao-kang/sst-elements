#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import unittest
import argparse
import test_globals
from test_support import *

REQUIRED_PYTHON_VER_MAJOR = 2 # Required Major Version
REQUIRED_PYTHON_VER_MINOR = 7 # Required Minor Version

#################################################

def validatePythonVersion():
    ver = sys.version_info
    if (ver[0] != REQUIRED_PYTHON_VER_MAJOR) or (ver[1] < REQUIRED_PYTHON_VER_MINOR):
        print("FATAL:\n{0} requires Python version {1}.{2}+;\nFound Python version is:\n{3}".format(os.path.basename(__file__),
                                                                                                    REQUIRED_PYTHON_VER_MAJOR,
                                                                                                    REQUIRED_PYTHON_VER_MINOR,
                                                                                                    sys.version))
        sys.exit(1)

def verifySSTCoreAndElementsAreAvailable():
    pass

#################################################

class TestEngine():
    def __init__(self):
        test_globals.initTestGlobals()
        test_globals.listOfSearchableTestPaths = ['.']
        self.sstFullTestSuite = unittest.TestSuite()
        self.failfast = False
        self._parseArguments()

    def _discoverTests(self):
        # Discover tests in each Test Path directory and add to the test suite
        sstPattern = 'testsuite*.py'
        for testpath in test_globals.listOfSearchableTestPaths:
            sstDiscoveredTests = unittest.TestLoader().discover(start_dir=testpath,
                                                                pattern=sstPattern,
                                                                top_level_dir = '.')
            self.sstFullTestSuite.addTests(sstDiscoveredTests)

        if test_globals.__TestAppDebug:
            print("\n")
            print("Searchable Test Paths = {0}".format(test_globals.listOfSearchableTestPaths))
            for test in self.sstFullTestSuite:
                print("Discovered Tests = {0}".format(test._tests))
            print("\n")

    def _createOutputDirectories(self):
        # Create the test output dir if necessary
        os.system("mkdir -p ./test_outputs")

    def runTests(self):
        self._createOutputDirectories()
        self._discoverTests()

        # Run all the tests
        sstTestsResults = unittest.TextTestRunner(verbosity=test_globals.verbosity,
                                                  failfast=self.failfast).run(self.sstFullTestSuite)

    def _parseArguments(self):
        parser = argparse.ArgumentParser(description='Run SST-Elements Integration Tests',
                                         epilog='Python files named TestScript*.py found at or below the defined test path(s) will be run.')
        parser.add_argument('listofpaths', metavar='test_path', nargs='*',
                            default=['.'], help='Directories to Test Suites [DEFAULT = .]')
        group = parser.add_mutually_exclusive_group()
        group.add_argument('-v', '--verbose', action='store_true',
                            help = 'Run tests in verbose mode')
        group.add_argument('-q', '--quiet', action='store_true',
                            help = 'Run tests in quiet mode')
        group.add_argument('-d', '--debug', action='store_true',
                            help='Run tests in test debug output mode')
        parser.add_argument('-r', '--ranks', type=int, metavar="XX",
                            nargs=1, default=0,
                            help='Run with XX Ranks')
        parser.add_argument('-t', '--threads', type=int, metavar="YY",
                            nargs=1, default=0,
                            help='Run with YY Threads')
        parser.add_argument('-f', '--failfast', action='store_true',
                            help = 'Stop on Failure')

        parser.add_argument('-x', '--testappdebug', action='store_true',
                            help = 'Enable Test Application Debug Mode')
                            #help = argparse.SUPPRESS)

        args = parser.parse_args()

        # Extract the Arguments into the class variables
        test_globals.verbosity = 1
        if args.debug == True:
            test_globals.verbosity = 3
            test_globals.debugMode = True
        if args.quiet == True:
            test_globals.verbosity = 0
        if args.verbose == True:
            test_globals.verbosity = 2
        test_globals.numRanks = args.ranks
        test_globals.numThreads = args.threads
        test_globals.listOfSearchableTestPaths = args.listofpaths
        self.failfast = args.failfast
        test_globals.__TestAppDebug = args.testappdebug

#################################################

def main():
    # Startup work
    validatePythonVersion()
    verifySSTCoreAndElementsAreAvailable()

    # Create the Test Engine and run it
    te = TestEngine()
    te.runTests()

#################################################
# Script Entry
#################################################
if __name__ == "__main__":
    # Call the main() subroutine
    main()

