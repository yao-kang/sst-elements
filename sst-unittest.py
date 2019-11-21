#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
#import os
#import datetime
#import re
import unittest

REQUIRED_PYTHON_VER_MAJOR = 2 # Required Major Version
REQUIRED_PYTHON_VER_MINOR = 7 # Required Minor Version

def validatePythonVersion():
    ver = sys.version_info
    if (ver[0] != REQUIRED_PYTHON_VER_MAJOR) or (ver[1] < REQUIRED_PYTHON_VER_MINOR):
        print("FATAL: Wrong version of Python - sst-unittest requires version {0}.{1}+; Found version is:\n{2}".format(REQUIRED_PYTHON_VER_MAJOR,
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

    # Implement Test Harness
    sstStartDir = '.'
    sstPattern = 'testcase*.py'
    sstTests = unittest.TestLoader().discover(start_dir = sstStartDir, pattern=sstPattern)
    sstTestsResults = unittest.TextTestRunner(verbosity=2).run(sstTests)

#################################################
# Script Entry
#################################################
if __name__ == "__main__":
    # Call the main() subroutine
    main()


