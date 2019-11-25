import os
import unittest
import filecmp

def testlog(stringtolog):
    print("\n0\n".format(stringtolog))


class test_merlin_Component(unittest.TestCase):

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
#        testlog("")

        # Set all the various file paths
        testDataFileName="test_merlin_{0}".format(testcase)

        sdlfile = "./src/sst/elements/merlin/tests/{0}.py".format(testcase)
        outfile = "./test_outputs/{0}.out".format(testDataFileName)
        reffile = "./src/sst/elements/merlin/tests/refFiles/test_merlin_{0}.out".format(testcase)

        # Build the launch command
        # TODO: Implement a run timeout
        oscmd = "sst {0} > {1}".format(sdlfile, outfile)
        os.system(oscmd)

        # Perform the test
        self.assertTrue(self.compare_sorted(outfile, reffile), "Output/Compare file {0} does not match Reference File {1}".format(outfile, reffile))


    def compare_sorted(self, outfile, reffile):
       sorted_outfile = "./test_outputs/sorted_outfile"
       sorted_reffile = "./test_outputs/sorted_reffile"

       os.system("sort -o {0} {1}".format(sorted_outfile, outfile))
       os.system("sort -o {0} {1}".format(sorted_reffile, reffile))

       return filecmp.cmp(sorted_outfile, sorted_reffile)








