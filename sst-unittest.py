#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import unittest

REQUIRED_PYTHON_VER_MAJOR = 2 # Required Major Version
REQUIRED_PYTHON_VER_MINOR = 7 # Required Minor Version

def validatePythonVersion():
    ver = sys.version_info
    if (ver[0] != REQUIRED_PYTHON_VER_MAJOR) or (ver[1] < REQUIRED_PYTHON_VER_MINOR):
        print("FATAL:\n{0} requires Python version {1}.{2}+;\nFound Python version is:\n{3}".format(os.path.basename(__file__),
                                                                                                    REQUIRED_PYTHON_VER_MAJOR,
                                                                                                    REQUIRED_PYTHON_VER_MINOR,
                                                                                                    sys.version))
        sys.exit(1)

def parsestartupargs():
    pass

def verifysstandelementsbuilt():
    pass


def main():
    # Startup work
    validatePythonVersion()
    parsestartupargs()
    verifysstandelementsbuilt()

    # Create the test output dir if necessary
    os.system("mkdir -p ./test_outputs")

    # Implement Test Harness
    sstStartDir = '.'
    sstPattern = 'testsuite*.py'
    sstTests = unittest.TestLoader().discover(start_dir = sstStartDir, pattern=sstPattern)
    sstTestsResults = unittest.TextTestRunner(verbosity=2).run(sstTests)

#################################################
# Script Entry
#################################################
if __name__ == "__main__":
    # Call the main() subroutine
    main()


