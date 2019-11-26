# -*- coding: utf-8 -*-

import os
import sys
import filecmp

import test_globals
from test_support import *

def setUpModule():
    initTestSuite(__file__)

def tearDownModule():
    pass

class test_simpleRNGComponent(SSTUnitTest):

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_RNG_Mersenne(self):
        self.RNG_test_template("mersenne")


    def test_RNG_Marsaglia(self):
        self.RNG_test_template("marsaglia")


    def test_RNG_xorshift(self):
        self.RNG_test_template("xorshift")

###

    def RNG_test_template(self, testcase):
        # Set the various file paths
        sdlfile = "{0}/test_simpleRNGComponent_{1}.py".format(self.getTestSuiteDir(), testcase)
        reffile = "{0}/refFiles/test_simpleRNGComponent_{1}.out".format(self.getTestSuiteDir(), testcase)
        outfile = "{0}/test_simpleRNGComponent_{1}.out".format(self.getTestOutputRunDir(), testcase)
        tmpfile = "{0}/test_simpleRNGComponent_{1}.tmp".format(self.getTestOutputRunDir(), testcase)
        cmpfile = "{0}/test_simpleRNGComponent_{1}.cmp".format(self.getTestOutputRunDir(), testcase)

        # Build the launch command
        # TODO: Implement a run timeout
        oscmd = "sst {0} > {1}".format(sdlfile, outfile)
        os.system(oscmd)

        # Post processing of the output data to scrub it into a format to compare
        os.system("grep Random {0} > {1}".format(outfile, tmpfile))
        os.system("tail -5 {0} > {1}".format(tmpfile, cmpfile))

        # Perform the test
        self.assertTrue(filecmp.cmp(cmpfile, reffile), "Output/Compare file {0} does not match Reference File {1}".format(cmpfile, reffile))