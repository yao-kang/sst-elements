# -*- coding: utf-8 -*-

import os
import sys
import filecmp

import test_globals
from test_support import *

def testlog(stringtolog):
    print("\n{0}\n".format(stringtolog))



def setUpModule():
    pass

def tearDownModule():
    pass

class test_simpleRNGComponent(SSTUnitTest):

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_RNG_Mersenne(self):
#        testlog("AARON - DEBUGMODE1 = {0}".format(test_globals.debugMode))
#        test_globals.debugMode = False
#        testlog("AARON - DEBUGMODE2 = {0}".format(test_globals.debugMode))
        self.RNG_test_template("mersenne")


    def test_RNG_Marsaglia(self):
        self.RNG_test_template("marsaglia")


    def test_RNG_xorshift(self):
        self.RNG_test_template("xorshift")

###

    def RNG_test_template(self, testcase):
#        testlog("")

        # Set all the various file paths
        sdlfile = "./src/sst/elements/simpleElementExample/tests/test_simpleRNGComponent_{0}.py".format(testcase)
        outfile = "./test_outputs/test_simpleRNGComponent_{0}.out".format(testcase)
        tmpfile = "./test_outputs/test_simpleRNGComponent_{0}.tmp".format(testcase)
        cmpfile = "./test_outputs/test_simpleRNGComponent_{0}.cmp".format(testcase)
        reffile = "./src/sst/elements/simpleElementExample/tests/refFiles/test_simpleRNGComponent_{0}.out".format(testcase)

        # Build the launch command
        # TODO: Implement a run timeout
        oscmd = "sst {0} > {1}".format(sdlfile, outfile)
        os.system(oscmd)

        # Post processing of the output data to scrub it into a format to compare
        os.system("grep Random {0} > {1}".format(outfile, tmpfile))
        os.system("tail -5 {0} > {1}".format(tmpfile, cmpfile))

        # Perform the test
        self.assertTrue(filecmp.cmp(cmpfile, reffile), "Output/Compare file {0} does not match Reference File {1}".format(cmpfile, reffile))