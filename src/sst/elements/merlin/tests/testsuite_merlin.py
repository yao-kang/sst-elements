# -*- coding: utf-8 -*-

import os
import filecmp

import test_globals
from test_support import *

def setUpModule():
    initTestSuite(__file__)

def tearDownModule():
    pass

class test_merlin_Component(SSTUnitTest):

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_merlin_dragon_128(self):
        self.merlin_test_template("dragon_128_test", 500)

    def test_merlin_dragon_72(self):
        self.merlin_test_template("dragon_72_test", 500)

    def test_merlin_fattree_128(self):
        self.merlin_test_template("fattree_128_test", 500)

    def test_merlin_fattree_256(self):
        self.merlin_test_template("fattree_256_test", 500)

    def test_merlin_torus_128(self):
        self.merlin_test_template("torus_128_test", 500)

    def test_merlin_torus_5_trafficgen(self):
        self.merlin_test_template("torus_5_trafficgen", 500)

    def test_merlin_torus_64(self):
         self.merlin_test_template("torus_64_test", 500)

############

    def merlin_test_template(self, testcase, tolerance):
        # Set the various file paths
        testDataFileName="test_merlin_{0}".format(testcase)

        sdlfile = "{0}/{1}.py".format(self.getTestSuiteDir(), testcase)
        reffile = "{0}/refFiles/test_merlin_{1}.out".format(self.getTestSuiteDir(), testcase)
        outfile = "{0}/{1}.out".format(self.getTestOutputDir(), testDataFileName)

        # Build the launch command
        # TODO: Implement a run timeout
        oscmd = "sst {0} > {1}".format(sdlfile, outfile)
        os.system(oscmd)

        # Perform the test
        cmp_result = self.compare_sorted(outfile, reffile)
        self.assertTrue(cmp_result, "Output/Compare file {0} does not match Reference File {1}".format(outfile, reffile))


    def compare_sorted(self, outfile, reffile):
       sorted_outfile = "{0}/sorted_outfile".format(self.getTestOutputDir())
       sorted_reffile = "{0}/sorted_reffile".format(self.getTestOutputDir())

       os.system("sort -o {0} {1}".format(sorted_outfile, outfile))
       os.system("sort -o {0} {1}".format(sorted_reffile, reffile))

       return filecmp.cmp(sorted_outfile, sorted_reffile)








